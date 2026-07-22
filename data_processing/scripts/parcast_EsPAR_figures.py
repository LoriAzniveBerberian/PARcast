#!/usr/bin/env python3
"""
PARcast - E_s(PAR) surface reference figures (all samples shown).

The surface SQ-500 is a standalone above-water logger: it records incident PAR
over time, with NO depth, NO immersion correction (it sits in air). This script
reads ONE surface file and writes figures to:
    <FIG_BASE>/E_sPAR/<shortname>_<datafile>.png

Figures: timeline, distribution, variability
(variability = how steady the sky was, which is the assumption the single-
 instrument E_d Kd fit leans on.)

Usage:
    python parcast_EsPAR_figures.py                     # uses ES_SURFACE_CSV below
    python parcast_EsPAR_figures.py /path/to/E_sPAR_x.CSV
    PARCAST_FIG_BASE=/somewhere python parcast_EsPAR_figures.py
"""
import os
import sys
import numpy as np
import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import matplotlib.dates as mdates

# ----------------------------------------------------------------------------
# CONFIG
# ----------------------------------------------------------------------------
ES_SURFACE_CSV = '/Users/loriberberian/Desktop/PARcast/data_processing/raw/E_sPAR/E_sPAR_20260608_0001.CSV'
FIG_BASE = os.environ.get(
    "PARCAST_FIG_BASE",
    "/Users/loriberberian/Desktop/PARcast/data_processing/figures",
)

# surface is IN AIR: factory sensitivity only, NO immersion correction
ES_SENSITIVITY = 100.0      # umol m-2 s-1 per mV (SQ-500)

# variability window for the sky-steadiness panel
VAR_WINDOW_S = 30.0         # seconds; rolling CV computed over this window

plt.rcParams.update({
    "figure.dpi": 110, "axes.grid": True, "grid.alpha": 0.25,
    "axes.spines.top": False, "axes.spines.right": False, "font.size": 10,
})

# ----------------------------------------------------------------------------
# OUTPUT FOLDER + FIGURE NAMING
# ----------------------------------------------------------------------------
if len(sys.argv) > 1:
    ES_SURFACE_CSV = sys.argv[1]

STEM   = os.path.splitext(os.path.basename(ES_SURFACE_CSV))[0]   # e.g. E_sPAR_20260608_0001
OUTDIR = os.path.join(FIG_BASE, "E_sPAR")
os.makedirs(OUTDIR, exist_ok=True)

def savefig(fig, shortname):
    path = os.path.join(OUTDIR, f"{shortname}_{STEM}.png")
    fig.savefig(path, bbox_inches="tight", dpi=150)
    plt.close(fig)
    print("saved", path)

# ----------------------------------------------------------------------------
# LOAD  (defensive: the surface logger's column names may differ)
# ----------------------------------------------------------------------------
ES_SOURCE = "mv"   # "mv" -> Es = millivolts * ES_SENSITIVITY ; "ppfd" -> trust logger PPFD column

def load_surface(path):
    df = pd.read_csv(path)
    # time column: prefer known names, else first column that parses as datetime
    tcol = None
    for c in (["iso_time", "datetime", "time", "timestamp"] + list(df.columns)):
        if c in df.columns:
            parsed = pd.to_datetime(df[c], errors="coerce")
            if parsed.notna().mean() > 0.5:
                tcol = c; df[c] = parsed; break
    if tcol is None:
        raise ValueError(f"no datetime column found (columns: {list(df.columns)})")
    # PAR columns: a millivolt column and/or an already-converted PPFD column
    mv_names   = ["par_mV", "par_mv", "parmV", "millivolts", "mV", "mv"]
    ppfd_names = ["par_uMol_m2_s", "ppfd_umol_m2_s", "ppfd", "par_uMol", "umol_m2_s"]
    mv_col   = next((c for c in mv_names   if c in df.columns), None)
    ppfd_col = next((c for c in ppfd_names if c in df.columns), None)
    df = df.rename(columns={tcol: "iso_time"})
    df = df.dropna(subset=["iso_time"]).sort_values("iso_time").reset_index(drop=True)
    if ES_SOURCE == "ppfd":
        if ppfd_col is None:
            raise ValueError(f"ES_SOURCE='ppfd' but no PPFD column (columns: {list(df.columns)})")
        df["Es"] = df[ppfd_col].astype(float); par_used = ppfd_col
    else:
        if mv_col is None:
            raise ValueError(f"ES_SOURCE='mv' but no millivolt column (columns: {list(df.columns)})")
        df["par_mV"] = df[mv_col].astype(float)
        df["Es"] = df["par_mV"] * ES_SENSITIVITY; par_used = mv_col
    n0 = len(df)
    df = df[df["Es"].notna() & (df["Es"] >= 0)].reset_index(drop=True)
    df.attrs["bad_rows_dropped"]  = n0 - len(df)
    df.attrs["time_col_detected"] = tcol
    df.attrs["par_col_detected"]  = f"{par_used} (ES_SOURCE={ES_SOURCE})"
    return df

# ============================================================================
# RUN
# ============================================================================
print(f"reading {ES_SURFACE_CSV}")
surf = load_surface(ES_SURFACE_CSV)
print(f"  detected time column '{surf.attrs['time_col_detected']}', "
      f"PAR column '{surf.attrs['par_col_detected']}'")
print(f"  {len(surf)} rows, {surf.attrs.get('bad_rows_dropped', 0)} bad rows dropped")
print(f"  Es {surf.Es.min():.0f}-{surf.Es.max():.0f} umol m-2 s-1, "
      f"median {surf.Es.median():.0f}")

# ---- Figure: timeline (all samples) ----------------------------------------
fig, ax = plt.subplots(figsize=(13, 4.5))
ax.plot(surf.iso_time, surf.Es, color="#e8a33d", lw=0.7)
ax.set_ylabel("E_s(PAR) incident PPFD\n(umol m-2 s-1)"); ax.set_xlabel("Time")
ax.set_title(f"Surface incident PAR  -  {STEM}")
ax.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
ax.margins(x=0.01)
fig.tight_layout(); savefig(fig, "timeline")

# ---- Figure: distribution (all samples) ------------------------------------
fig, ax = plt.subplots(figsize=(7.5, 5))
ax.hist(surf.Es, bins=60, color="#e8a33d", edgecolor="#a8721f", linewidth=0.4)
ax.axvline(surf.Es.median(), color="#c0392b", lw=1.5, label=f"median {surf.Es.median():.0f}")
ax.set_xlabel("E_s(PAR) incident PPFD (umol m-2 s-1)"); ax.set_ylabel("count")
ax.set_title("Surface PAR distribution (all samples)")
ax.legend(fontsize=9)
fig.tight_layout(); savefig(fig, "distribution")

# ---- Figure: variability (rolling CV = sky steadiness) ---------------------
# Median sample spacing -> window length in samples
dt_s = surf.iso_time.diff().dt.total_seconds().median()
win = max(3, int(round(VAR_WINDOW_S / dt_s))) if dt_s and dt_s > 0 else 30
roll = surf.Es.rolling(win, center=True, min_periods=max(3, win // 3))
cv = 100.0 * roll.std() / roll.mean()

fig, (axT, axB) = plt.subplots(2, 1, figsize=(13, 7), sharex=True)
axT.plot(surf.iso_time, surf.Es, color="#e8a33d", lw=0.7)
axT.set_ylabel("E_s(PAR)\nincident PPFD")
axT.set_title(f"Surface PAR and short-term variability  (window = {win} samples ~ {VAR_WINDOW_S:.0f}s)")
axB.plot(surf.iso_time, cv, color="#6c3483", lw=0.8)
axB.set_ylabel("rolling CV (%)"); axB.set_xlabel("Time")
axB.axhline(5, color="red", ls=":", lw=1, label="5% (fairly steady sky)")
axB.legend(fontsize=8)
axB.xaxis.set_major_formatter(mdates.DateFormatter("%H:%M"))
for a in (axT, axB): a.margins(x=0.01)
fig.tight_layout(); savefig(fig, "variability")

print(f"  median short-term CV = {np.nanmedian(cv):.1f}%  "
      "(low = steady sky -> single-instrument Kd assumption holds better)")
print("done.")
