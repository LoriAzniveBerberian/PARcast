#!/usr/bin/env python3
"""
PARcast - E_d(z,PAR) in-water profiler figures (no binning, all samples shown).

Reads ONE in-water file and writes figures to:
    <FIG_BASE>/E_dzPAR/<shortname>_<datafile>.png

Figures: timeline, depth_profile, temp_profile, kd_fit, logx_profile,
         percast_fits, kd_variability

Usage:
    python parcast_EdzPAR_figures.py                      # uses ED_PROFILE_CSV below
    python parcast_EdzPAR_figures.py /path/to/E_dzPAR_x.CSV
    PARCAST_FIG_BASE=/somewhere python parcast_EdzPAR_figures.py
"""
import os
import sys
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")                       # headless: write PNGs, no window
import matplotlib.pyplot as plt
import matplotlib.dates as mdates
from scipy import stats

# ----------------------------------------------------------------------------
# CONFIG
# ----------------------------------------------------------------------------
ED_PROFILE_CSV = '/Users/loriberberian/Desktop/PARcast/data_processing/raw/E_dzPAR/E_dzPAR_20260608_0001.CSV'
FIG_BASE = os.environ.get(
    "PARCAST_FIG_BASE",
    "/Users/loriberberian/Desktop/PARcast/data_processing/figures",
)

# calibration: rebuild PPFD from RAW par_mV (firmware par_uMol multiplier is wrong)
ED_SENSITIVITY  = 100.0     # umol m-2 s-1 per mV  (SQ-500 factory sensitivity)
IECF            = 1.32      # Apogee immersion correction, SQ-500 (1.08 for SQ-120)
APPLY_IMMERSION = True      # False -> show the pre-immersion (relative) scale

# analysis settings
TILT_OK_DEG = 5.0           # cosine collector trusted below this tilt (deg)
MIN_ED      = 50.0          # umol m-2 s-1: below this the sensor is effectively dark

# cast isolation
CAST_MODE       = "auto"    # "auto" | "manual" | "all"
CAST_START      = None      # "2026-06-04 13:08" (manual only)
CAST_END        = None      # "2026-06-04 13:20"
CAST_ACTIVITY_M = 0.15      # auto: min rolling depth std (m) to count as moving
CAST_DIRECTION  = "down"    # "down" | "up" | "both"

# what counts as a real profiling cast (auto mode). These reject surface bobbing
# and parked/capped periods (e.g. the instrument sitting in a box on deck taking
# dark readings): a cast must actually reach depth and last a few seconds.
CAST_MIN_DEPTH_M    = 1.0   # a cast must reach at least this deep
CAST_MIN_DURATION_S = 5.0   # ...and last at least this long
CAST_BRIDGE_GAP_S   = 3.0   # merge segments split by a pause shorter than this

# per-cast Kd fit (no binning - regress on all samples in each cast)
MIN_PTS_PER_CAST  = 30      # cast needs at least this many lit samples
MIN_SPAN_M        = 1.0     # ... and this much depth span

IMU_UP_AXIS = "accel_y_ms2"
DEPTH_MIN_M, TEMP_MIN_C = -10.0, -10.0

plt.rcParams.update({
    "figure.dpi": 110, "axes.grid": True, "grid.alpha": 0.25,
    "axes.spines.top": False, "axes.spines.right": False, "font.size": 10,
})

# ----------------------------------------------------------------------------
# OUTPUT FOLDER + FIGURE NAMING
# ----------------------------------------------------------------------------
if len(sys.argv) > 1:
    ED_PROFILE_CSV = sys.argv[1]

STEM   = os.path.splitext(os.path.basename(ED_PROFILE_CSV))[0]   # e.g. E_dzPAR_20260608_0001
OUTDIR = os.path.join(FIG_BASE, "E_dzPAR")
os.makedirs(OUTDIR, exist_ok=True)

def savefig(fig, shortname):
    """Write <shortname>_<datafile>.png into the E_dzPAR figures folder."""
    path = os.path.join(OUTDIR, f"{shortname}_{STEM}.png")
    fig.savefig(path, bbox_inches="tight", dpi=150)
    plt.close(fig)
    print("saved", path)

# ----------------------------------------------------------------------------
# LOAD / CLEAN / CAST DETECTION  (unchanged from the notebook)
# ----------------------------------------------------------------------------
def compute_tilt(df):
    mag = np.sqrt(df.accel_x_ms2**2 + df.accel_y_ms2**2 + df.accel_z_ms2**2)
    return np.degrees(np.arccos(np.clip(df[IMU_UP_AXIS] / mag, -1, 1)))

def load_profile(path):
    df = pd.read_csv(path, parse_dates=["iso_time"]).sort_values("iso_time").reset_index(drop=True)
    n0 = len(df)
    df = df[(df.depth_m > DEPTH_MIN_M) & (df.water_temp_C > TEMP_MIN_C)
            & (df.pressure_mbar > 0)].reset_index(drop=True)
    df.attrs["glitch_rows_dropped"] = n0 - len(df)
    df["tilt_deg"] = compute_tilt(df)
    immersion = IECF if APPLY_IMMERSION else 1.0
    df["Ed"] = df["par_mV"] * ED_SENSITIVITY * immersion
    return df

def add_cast_flag(profile, activity_m=CAST_ACTIVITY_M, window=80):
    p = profile.copy()
    p["depth_activity"] = p["depth_m"].rolling(window, center=True, min_periods=20).std()
    p["profiling"] = p["depth_activity"] > activity_m
    dt = p["iso_time"].diff().dt.total_seconds().replace(0, np.nan)
    p["vspeed"] = p["depth_m"].diff() / dt
    vs = p["vspeed"].rolling(window, center=True, min_periods=10).median()
    p["direction"] = np.where(vs >= 0, "down", "up")
    return p

def find_casts(profile, activity_m=CAST_ACTIVITY_M, window=80,
               min_depth=CAST_MIN_DEPTH_M, min_dur_s=CAST_MIN_DURATION_S,
               bridge_gap_s=CAST_BRIDGE_GAP_S):
    """Identify discrete profiling casts. A cast is a contiguous stretch where the
    instrument is moving through the column AND actually reaches depth. This rejects
    surface bobbing (never gets deep) and parked/capped periods such as the unit
    sitting in a box on deck (flat depth near the surface, often dark).
    Returns a list of dicts: t0, t1, zmax, dur_s, n."""
    p = add_cast_flag(profile, activity_m, window).reset_index(drop=True)
    moving = p["profiling"].to_numpy()
    n = len(p)
    runs, i = [], 0
    while i < n:                                   # contiguous runs of "moving"
        if not moving[i]:
            i += 1; continue
        j = i
        while j + 1 < n and moving[j + 1]:
            j += 1
        runs.append([i, j]); i = j + 1
    merged = []                                    # bridge short pauses mid-cast
    for a, b in runs:
        if merged:
            gap = (p["iso_time"].iloc[a] - p["iso_time"].iloc[merged[-1][1]]).total_seconds()
            if gap <= bridge_gap_s:
                merged[-1][1] = b; continue
        merged.append([a, b])
    casts = []                                     # keep only real, deep-enough casts
    for a, b in merged:
        seg = p.iloc[a:b + 1]
        dur = (seg["iso_time"].iloc[-1] - seg["iso_time"].iloc[0]).total_seconds()
        if seg["depth_m"].max() >= min_depth and dur >= min_dur_s:
            casts.append(dict(t0=seg["iso_time"].iloc[0], t1=seg["iso_time"].iloc[-1],
                              zmax=seg["depth_m"].max(), dur_s=dur, n=len(seg)))
    return casts

def in_windows(times, windows):
    """Boolean mask: which timestamps fall inside any (t0, t1) cast window."""
    tv = pd.to_datetime(pd.Series(times)).values
    m = np.zeros(len(tv), bool)
    for w0, w1 in windows:
        m |= (tv >= np.datetime64(w0)) & (tv <= np.datetime64(w1))
    return m

def select_cast(profile, mode="auto", start=None, end=None,
                activity_m=CAST_ACTIVITY_M, direction="both"):
    """Return (cast_rows, windows). `windows` is a list of (t0, t1) — one per cast —
    so the timeline can shade each cast separately instead of one big span."""
    p = add_cast_flag(profile, activity_m)
    if mode == "manual" and start and end:
        windows = [(pd.Timestamp(start), pd.Timestamp(end))]
    elif mode == "all":
        windows = [(p["iso_time"].min(), p["iso_time"].max())]
    else:                                          # auto
        windows = [(c["t0"], c["t1"]) for c in find_casts(profile, activity_m)]
        if not windows:                            # fallback: nothing detected
            windows = [(p["iso_time"].min(), p["iso_time"].max())]
    cast = p[in_windows(p["iso_time"], windows)].copy()
    if direction in ("down", "up"):
        cast = cast[cast["direction"] == direction].copy()
    return cast, windows

# ----------------------------------------------------------------------------
# PER-CAST Kd  (each cast = one replicate; NO depth binning - fit all samples)
# ----------------------------------------------------------------------------
def fit_kd_segment(seg, value_col="Ed", min_val=MIN_ED, min_n=MIN_PTS_PER_CAST):
    """Regress ln(value) on depth for ONE cast using ALL lit samples (no bins)."""
    s = seg[seg[value_col] > min_val].copy()
    if len(s) < min_n or (s.depth_m.max() - s.depth_m.min()) < MIN_SPAN_M:
        return None
    s["lnE"] = np.log(s[value_col])
    f = stats.linregress(s.depth_m, s.lnE)
    return dict(Kd=-f.slope, r2=f.rvalue**2, slope_se=f.stderr, n_pts=len(s),
                z0=s.depth_m.min(), z1=s.depth_m.max(),
                slope=f.slope, intercept=f.intercept, samples=s)

def fit_kd_per_cast(profile, direction=CAST_DIRECTION, value_col="Ed",
                    activity_m=CAST_ACTIVITY_M):
    """Fit Kd for each detected cast (one direction leg per cast). Iterates the same
    depth-validated casts as find_casts, so the box/dark period is never fit."""
    p = add_cast_flag(profile, activity_m)
    casts = find_casts(profile, activity_m)
    tv = p["iso_time"].values
    rows, fits = [], {}
    for c in casts:
        seg = p[(tv >= np.datetime64(c["t0"])) & (tv <= np.datetime64(c["t1"]))]
        if direction in ("down", "up"):
            seg = seg[seg["direction"] == direction]
        r = fit_kd_segment(seg, value_col=value_col)
        if r is None:
            continue
        lab = len(rows) + 1; fits[lab] = r
        rows.append(dict(cast=lab, t_start=c["t0"], zmax=c["zmax"], z0=r["z0"], z1=r["z1"],
                         n_pts=r["n_pts"], tilt_med=seg["tilt_deg"].median(),
                         Kd=r["Kd"], r2=r["r2"], slope_se=r["slope_se"]))
    return pd.DataFrame(rows), fits

def kd_ensemble_stats(tbl):
    """Honest Kd: each cast = one replicate, uncertainty = spread ACROSS casts."""
    k = tbl["Kd"].to_numpy(); n = len(k)
    if n == 0:
        return {}
    mean = k.mean(); sd = k.std(ddof=1) if n > 1 else np.nan
    se = sd / np.sqrt(n) if n > 1 else np.nan
    tcrit = stats.t.ppf(0.975, n - 1) if n > 1 else np.nan
    return dict(n=n, mean=mean, sd=sd, se=se,
                ci_lo=mean - tcrit * se if n > 1 else np.nan,
                ci_hi=mean + tcrit * se if n > 1 else np.nan,
                median=float(np.median(k)),
                cv=100 * sd / mean if n > 1 else np.nan)

# ============================================================================
# RUN
# ============================================================================
print(f"reading {ED_PROFILE_CSV}")
profile = load_profile(ED_PROFILE_CSV)
cast, windows = select_cast(profile, CAST_MODE, CAST_START, CAST_END,
                            CAST_ACTIVITY_M, CAST_DIRECTION)
cast_t0, cast_t1 = windows[0][0], windows[-1][1]   # overall span (for reference)
print(f"  {len(profile)} rows, {profile.attrs.get('glitch_rows_dropped', 0)} glitches dropped")
print(f"  depth {profile.depth_m.min():.2f}-{profile.depth_m.max():.2f} m, "
      f"cast {CAST_DIRECTION}: {len(windows)} casts, {len(cast)} rows, "
      f"tilt median {cast.tilt_deg.median():.0f} deg")

# ---- Figure: timeline ------------------------------------------------------
fig, ax = plt.subplots(4, 1, figsize=(13, 10), sharex=True)
ax[0].plot(profile.iso_time, profile.Ed, color="#c0392b", lw=0.7)
for _i, (w0, w1) in enumerate(windows):
    ax[0].axvspan(w0, w1, color="#2e6f95", alpha=0.18, label="profiling cast" if _i == 0 else None)
ax[0].set_ylabel("E_d(PAR)\nin-water PPFD"); ax[0].legend(loc="upper right", fontsize=8)
ax[1].plot(profile.iso_time, profile.depth_m, color="#2e6f95", lw=0.7)
for w0, w1 in windows:
    ax[1].axvspan(w0, w1, color="#2e6f95", alpha=0.18)
ax[1].invert_yaxis(); ax[1].set_ylabel("Depth (m)")
ax[2].plot(profile.iso_time, profile.water_temp_C, color="#1e8449", lw=0.7)
for w0, w1 in windows:
    ax[2].axvspan(w0, w1, color="#2e6f95", alpha=0.18)
ax[2].set_ylabel("Water T (C)")
ax[3].plot(profile.iso_time, profile.tilt_deg, color="#6c3483", lw=0.6)
ax[3].axhline(TILT_OK_DEG, color="red", ls=":", lw=1, label=f"{TILT_OK_DEG:.0f} deg limit")
ax[3].set_ylabel("Tilt (deg)"); ax[3].set_xlabel("Time"); ax[3].legend(loc="upper right", fontsize=8)
ax[3].xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
for a in ax: a.margins(x=0.01)
fig.suptitle(f"In-water profiler deployment  -  {STEM}", y=0.995)
fig.tight_layout(); savefig(fig, "timeline")

# ---- Figure: depth_profile (all samples, coloured by tilt, NO bin line) ----
fig, ax = plt.subplots(figsize=(6.5, 8))
parked = profile[~in_windows(profile.iso_time, windows)]
ax.scatter(parked.Ed, parked.depth_m, s=3, c="#dddddd", label="parked / drifting", zorder=1)
tilted = cast[cast.tilt_deg >= TILT_OK_DEG]
level  = cast[cast.tilt_deg <  TILT_OK_DEG]
ax.scatter(tilted.Ed, tilted.depth_m, s=5, c="#bbbbbb",
           label=f"cast, tilt >= {TILT_OK_DEG:.0f} deg", zorder=2)
sc = ax.scatter(level.Ed, level.depth_m, s=7, c=level.tilt_deg, cmap="viridis",
                vmin=0, vmax=TILT_OK_DEG, label=f"cast, tilt < {TILT_OK_DEG:.0f} deg", zorder=3)
ax.invert_yaxis()
ax.set_xlabel("E_d(PAR) in-water PPFD (umol m-2 s-1)"); ax.set_ylabel("Depth (m)")
ax.set_title(f"In-water PAR depth profile  (cast: {CAST_DIRECTION})")
fig.colorbar(sc, ax=ax, label="tilt (deg)", shrink=0.6)
ax.legend(fontsize=8, loc="lower right")
fig.tight_layout(); savefig(fig, "depth_profile")

# ---- Figure: temp_profile (all samples, coloured by time, NO bin line) -----
fig, ax = plt.subplots(figsize=(6.5, 8))
t_rel = (profile.iso_time - profile.iso_time.min()).dt.total_seconds()
sc = ax.scatter(profile.water_temp_C, profile.depth_m, s=6, c=t_rel, cmap="cividis")
ax.invert_yaxis()
ax.set_xlabel("Water temperature (C)"); ax.set_ylabel("Depth (m)")
ax.set_title("Temperature profile (all samples)")
fig.colorbar(sc, ax=ax, label="seconds into deployment", shrink=0.6)
fig.tight_layout(); savefig(fig, "temp_profile")

# ---- Figure: kd_fit (pooled, fit on ALL cast samples, no bins) -------------
good = cast[cast.Ed > MIN_ED].copy()
good["lnEd"] = np.log(good.Ed)
fig, ax = plt.subplots(figsize=(7.5, 6))
Kd = None
if len(good) >= 3 and (good.depth_m.max() - good.depth_m.min()) > MIN_SPAN_M:
    fit = stats.linregress(good.depth_m, good.lnEd)
    Kd = -fit.slope
    xs = np.linspace(good.depth_m.min(), good.depth_m.max(), 50)
    ax.plot(xs, fit.intercept + fit.slope * xs, color="#c0392b", lw=2,
            label=f"Kd = {Kd:.3f} /m   (R2 = {fit.rvalue**2:.3f}, all samples)")
ax.scatter(good.depth_m, good.lnEd, s=5, c="#2e6f95", alpha=0.35, label="all cast samples")
ax.set_xlabel("Depth (m)"); ax.set_ylabel("ln( E_d )")
ax.set_title("Diffuse attenuation Kd(PAR) - pooled fit, all samples")
ax.legend(fontsize=9)
fig.tight_layout(); savefig(fig, "kd_fit")
if Kd is not None:
    print(f"  pooled Kd(PAR) = {Kd:.3f} /m   (R2={fit.rvalue**2:.3f}; nominal SE is optimistic - "
          "samples autocorrelated, use the across-cast spread below)")

# ---- Per-cast fits for the honest uncertainty ------------------------------
kd_table, kd_fits = fit_kd_per_cast(profile, direction=CAST_DIRECTION, value_col="Ed")
kd_stats = kd_ensemble_stats(kd_table)
_cmap = plt.get_cmap("turbo")
_N = max(1, len(kd_table))
cast_color = {row.cast: _cmap(i / max(1, _N - 1)) for i, row in enumerate(kd_table.itertuples())}

# ---- Figure: logx_profile (all samples, one colour per cast) ---------------
fig, ax = plt.subplots(figsize=(6.5, 8))
for cid, r in kd_fits.items():
    s = r["samples"]
    ax.scatter(s["Ed"], s.depth_m, s=6, color=cast_color[cid], alpha=0.55, label=f"cast {cid}")
ax.set_xscale("log"); ax.invert_yaxis()
ax.set_xlabel("E_d(PAR) in-water PPFD (log scale)"); ax.set_ylabel("Depth (m)")
ax.set_title("Log-x depth profile by cast (all samples)")
if len(kd_fits): ax.legend(fontsize=8, loc="lower left")
fig.tight_layout(); savefig(fig, "logx_profile")

# ---- Figure: percast_fits (all samples + per-cast line; residuals) ---------
fig, (axL, axR) = plt.subplots(1, 2, figsize=(13, 6))
for cid, r in kd_fits.items():
    s = r["samples"]
    axL.scatter(s.depth_m, s.lnE, s=5, color=cast_color[cid], alpha=0.30)
    xs = np.linspace(s.depth_m.min(), s.depth_m.max(), 30)
    axL.plot(xs, r["intercept"] + r["slope"] * xs, color=cast_color[cid], lw=1.8,
             label=f"cast {cid}: Kd={r['Kd']:.3f}  R2={r['r2']:.3f}")
    axR.scatter(s.depth_m, s.lnE - (r["intercept"] + r["slope"] * s.depth_m),
                s=5, color=cast_color[cid], alpha=0.30)
axL.set_xlabel("Depth (m)"); axL.set_ylabel("ln( E_d )"); axL.set_title("Per-cast fits (all samples)")
if len(kd_fits): axL.legend(fontsize=8)
axR.axhline(0, color="k", lw=0.8, ls=":")
axR.set_xlabel("Depth (m)"); axR.set_ylabel("residual ln( E_d )")
axR.set_title("Linearity check (fit residuals)")
fig.tight_layout(); savefig(fig, "percast_fits")

# ---- Figure: kd_variability ------------------------------------------------
fig, (ax1, ax2, ax3) = plt.subplots(1, 3, figsize=(15, 5))
if len(kd_table):
    ax1.errorbar(kd_table.cast, kd_table.Kd, yerr=kd_table.slope_se, fmt="o",
                 color="#2e6f95", capsize=3, label="per-cast Kd (± within-cast SE)")
    if kd_stats["n"] > 1:
        ax1.axhline(kd_stats["mean"], color="#c0392b", lw=2, label=f"mean {kd_stats['mean']:.3f}/m")
        ax1.axhspan(kd_stats["ci_lo"], kd_stats["ci_hi"], color="#c0392b", alpha=0.15,
                    label=f"95% CI [{kd_stats['ci_lo']:.3f}, {kd_stats['ci_hi']:.3f}]")
ax1.set_xlabel("cast #"); ax1.set_ylabel("Kd (/m)"); ax1.set_title("Kd per cast"); ax1.legend(fontsize=8)
if len(kd_table):
    ax2.scatter(kd_table.tilt_med, kd_table.Kd, s=45, color="#6c3483")
ax2.axvline(TILT_OK_DEG, color="red", ls=":", lw=1, label=f"{TILT_OK_DEG:.0f} deg limit")
ax2.set_xlabel("cast median tilt (deg)"); ax2.set_ylabel("Kd (/m)")
ax2.set_title("Kd vs cast tilt"); ax2.legend(fontsize=8)
if len(kd_table):
    ax3.boxplot(kd_table.Kd, widths=0.5)
    ax3.scatter(np.ones(len(kd_table)) + np.random.normal(0, 0.03, len(kd_table)),
                kd_table.Kd, color="#2e6f95", zorder=3)
ax3.set_xticks([1]); ax3.set_xticklabels(["all casts"]); ax3.set_ylabel("Kd (/m)")
ax3.set_title(f"Kd spread (CV = {kd_stats.get('cv', float('nan')):.1f}%)")
fig.tight_layout(); savefig(fig, "kd_variability")

if kd_stats.get("n", 0) > 1:
    print(f"  per-cast Kd = {kd_stats['mean']:.3f} +/- {kd_stats['sd']:.3f} /m "
          f"(SD across {kd_stats['n']} casts); 95% CI [{kd_stats['ci_lo']:.3f}, {kd_stats['ci_hi']:.3f}]")
elif kd_stats.get("n", 0) == 1:
    print("  only one qualifying cast - need replicate casts for an honest error bar")
else:
    print("  no qualifying casts - lower MIN_PTS_PER_CAST / MIN_SPAN_M or set CAST_DIRECTION='both'")

print("done.")
