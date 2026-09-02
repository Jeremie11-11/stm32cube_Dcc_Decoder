# Specification

## Initial speed limit

When the decoder starts without valid backup data, `signal_state` and `signal_state_active` are initialized to `SIGNAL_FREE`. The speed limit is initialized from the active state.

`signal_state_active` always contains a valid limiting state: `SIGNAL_STOP`, `SIGNAL_40KMH`, `SIGNAL_60KMH`, `SIGNAL_90KMH`, or `SIGNAL_FREE`. A detected `SIGNAL_NOCHANGE` does not replace the active state.

The active signal state is stored in the backup. On restoration, `signal_state`, `signal_state_active`, and `speed_limit` are initialized from that saved state.

## Stop zone exit

When a stable `SIGNAL_NOCHANGE` is detected while the decoder is in `SIGNAL_STOP`, the signal state transitions to `SIGNAL_FREE`. This prevents the decoder from remaining stuck without a red signal.

`speed_limit = 0` is only allowed while `SIGNAL_STOP` is active.
