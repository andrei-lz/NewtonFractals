#!/usr/bin/env bash
set -euo pipefail

# --- Config (can be overridden via env) ---
: "${IMG:=1536x1536}"
: "${ITERS:=400}"
: "${TOL:=1e-12}"
: "${POLY:=z3-1}"
: "${BOUNDS:="-2 2 -2 2"}"
: "${BIN:=./newton_fractals}"

# --- Sanity checks ---
if [[ ! -x "$BIN" ]]; then
  echo "ERROR: binary not found or not executable: $BIN" >&2
  exit 1
fi

# Get supported poly IDs from --help output (second line usually lists them)
help_out="$("$BIN" --help 2>&1 || true)"
if ! grep -qE -- "--poly.*\(" <<<"$help_out"; then
  echo "WARN: couldn't parse supported polys from --help, proceeding..." >&2
else
  if ! grep -qE -- "\b$POLY\b" <<<"$help_out"; then
    echo "ERROR: POLY='$POLY' not in supported set:" >&2
    echo "$help_out" | sed -n '1,6p' >&2
    exit 1
  fi
fi

# --- Header ---
echo "cores,size,iters,seconds"

# --- Run ---
for t in 1 2 4 8 16; do
  # Use GNU time for wall clock; capture stderr (time prints to stderr)
  # Also capture program stderr/stdout to a log for debugging if it fails.
  log="/tmp/bench_${t}.log"
  : > "$log"

  # Run and measure. If the program exits non-zero, we still print why.
  sec="$(
    /usr/bin/time -f "%e" \
      env OMP_NUM_THREADS="$t" "$BIN" \
        --poly "$POLY" \
        --size "$IMG" \
        --max-iters "$ITERS" \
        --tol "$TOL" \
        --bounds $BOUNDS \
        --threads "$t" \
        --out /tmp/bench \
        >"$log" 2>&1 \
      || { echo "RUNFAIL" ; }
  )"

  if [[ "$sec" == "RUNFAIL" ]]; then
    echo "ERROR: run failed for threads=$t. Log follows:" >&2
    sed -n '1,120p' "$log" >&2
    continue
  fi

  # Some shells append the whole log; ensure we only got a number
  if [[ ! "$sec" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
    # If the app printed usage, show it and skip this row
    if grep -qE "^newton_fractals" "$log"; then
      echo "WARN: program printed usage for threads=$t. Check flags/POLY. Snippet:" >&2
      sed -n '1,12p' "$log" >&2
      continue
    fi
    echo "WARN: unexpected timing output for threads=$t: '$sec' — raw log:" >&2
    sed -n '1,60p' "$log" >&2
    continue
  fi

  echo "$t,$IMG,$ITERS,$sec"
done
