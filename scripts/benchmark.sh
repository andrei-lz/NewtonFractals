#!/usr/bin/env bash
set -euo pipefail

: "${IMG:=1536x1536}"
: "${ITERS:=400}"
: "${TOL:=1e-12}"
: "${POLY:=z3-1}"
: "${BOUNDS:="-2 2 -2 2"}"
: "${BIN:=./newton_fractals}"

# --- pick a timing command: gtime (Homebrew), GNU time, or fall back to bash 'time -p'
pick_time() {
  if command -v gtime >/dev/null 2>&1; then echo "gtime"; return; fi
  if command -v /usr/bin/time >/dev/null 2>&1; then echo "/usr/bin/time"; return; fi
  echo "builtin"
}
TIME_CMD="$(pick_time)"

# --- sanity
[[ -x "$BIN" ]] || { echo "ERROR: binary not executable: $BIN" >&2; exit 1; }
help_out="$("$BIN" --help 2>&1 || true)"
if ! grep -qE "\b$POLY\b" <<<"$help_out"; then
  echo "ERROR: POLY='$POLY' not supported. Available:" >&2
  echo "$help_out" | sed -n '1,8p' >&2
  exit 1
fi

echo "cores,size,iters,seconds"

for t in 1 2 4 8 16; do
  log="/tmp/bench_${t}.log"
  : > "$log"

  if [[ "$TIME_CMD" == "builtin" ]]; then
    # Use bash's 'time -p' → prints "real <sec>" to stderr
    # Capture only the timing line; program stdout/stderr go to $log
    timing="$({ time -p env OMP_NUM_THREADS="$t" "$BIN" \
                  --poly "$POLY" --size "$IMG" --max-iters "$ITERS" --tol "$TOL" \
                  --bounds $BOUNDS --threads "$t" --out /tmp/bench \
                  >"$log" 2>&1; } 2>&1 || echo RUNFAIL)"
    if [[ "$timing" == "RUNFAIL" ]]; then
      echo "ERROR: run failed for threads=$t. Log follows:" >&2
      sed -n '1,120p' "$log" >&2
      continue
    fi
    sec="$(awk '/^real/{print $2}' <<<"$timing")"
  else
    # GNU time or gtime: '-f %e' gives elapsed seconds
    # It writes to stderr; we capture it while program output goes to $log
    sec="$("$TIME_CMD" -f '%e' env OMP_NUM_THREADS="$t" "$BIN" \
            --poly "$POLY" --size "$IMG" --max-iters "$ITERS" --tol "$TOL" \
            --bounds $BOUNDS --threads "$t" --out /tmp/bench \
            >"$log" 2>&1 || echo RUNFAIL)"
    if [[ "$sec" == "RUNFAIL" ]]; then
      echo "ERROR: run failed for threads=$t. Log follows:" >&2
      sed -n '1,120p' "$log" >&2
      continue
    fi
    # some shells echo the whole log; keep only a numeric like 1.234
    sec="$(grep -Eo '^[0-9]+([.][0-9]+)?$' <<<"$sec" || true)"
  fi

  if [[ -z "$sec" ]]; then
    echo "WARN: no timing parsed for threads=$t. Snippet of log:" >&2
    sed -n '1,15p' "$log" >&2
    continue
  fi

  echo "$t,$IMG,$ITERS,$sec"
done
