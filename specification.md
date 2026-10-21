# Specification

## DCC command confirmation

An addressed or broadcast DCC command is executed only after two consecutive identical relevant frames with valid XOR checksums. Frames for other nodes do not interrupt confirmation. A different addressed or broadcast frame replaces the candidate; further identical frames are ignored after confirmation.

At the existing received-message counter multiple of 400, the candidate and confirmation are cleared, and two matching frames are required again. Addressed and broadcast frames reset the counter to 1, including unconfirmed frames and duplicates. The existing uplink stop at expiration is preserved; an addressed command starts uplink only after confirmation, while broadcast commands do not start it.

## Initial speed limit

When the decoder starts without valid backup data, `signal_state` and `signal_state_active` are initialized to `SIGNAL_FREE`. The speed limit is initialized from the active state.

`signal_state_active` always contains a valid limiting state: `SIGNAL_STOP`, `SIGNAL_40KMH`, `SIGNAL_60KMH`, `SIGNAL_90KMH`, or `SIGNAL_FREE`. A detected `SIGNAL_NOCHANGE` does not replace the active state.

The detected signal state, active limiting state, and remaining hold time are stored in the backup. Restoration preserves these states and initializes `speed_limit` from the saved active state. The previous direction is initialized from the restored actual direction, so restoration does not trigger a false direction change.

## Motor stop and backup lifetime

The motor has three states: RUN, STOPPING, and IDLE. In RUN, the operating backup is refreshed at each motor speed update. When the speed reference reaches zero, the H-bridge is disabled and the motor enters STOPPING for the existing two-second restart delay. No restart is permitted during this immobilization period, and the last running backup remains valid without being overwritten by a stopped state.

When the delay expires, the motor transitions from STOPPING to IDLE and invalidates the backup. A new start is then permitted and backup updates resume in RUN. A reset during STOPPING can restore the last running backup; a reset in IDLE follows normal startup without restoring backup registers. Invalidation does not erase configuration or change the current in-RAM signal limit. The backup format is unchanged. Immobilization is defined by the speed reference and delay, not by a physical standstill measurement.

## Speed limit hold time

Configuration word 15 (byte offset 60), previously unused, stores `signal_hold_60kmh_s`: the hold time in seconds for a 60 km/h limit. Its default value is zero. Reference values above 36000 seconds are treated as 36000 seconds.

While a fixed 40, 60, or 90 km/h limit is detected, the hold timer is continuously reloaded. The corresponding durations are `T60 * 3 / 2`, `T60`, and `T60 * 2 / 3`, rounded down to whole seconds.

In `SIGNAL_NOCHANGE`, the timer is no longer reloaded. The last limit remains active until the timer expires, then the active state becomes `SIGNAL_FREE` with a limit of 320 km/h. A zero hold time releases the limit on the first speed update in `SIGNAL_NOCHANGE`. A newly detected fixed limit reloads the timer with its corresponding duration.

Hold durations are calculated only at startup. Changes to the configured reference duration take effect after restarting the decoder. While the motor is in RUN, the remaining hold time is saved in backup registers in milliseconds at each motor speed update. After restoration in No Change, the countdown resumes from the last saved remaining time rather than restarting a full duration. Time without power is not deducted. Without valid backup data, the timer starts at zero. `SIGNAL_FREE` clears the timer; the immediate release from STOP to NOCHANGE remains unchanged.

## Direction change

Changing the actual direction from forwards to backwards or vice versa releases the stored speed limit in No Change zones, returning it to 320 km/h. A currently detected fixed restriction, including STOP, remains authoritative. Initializing an unknown direction is not treated as a direction change.

## Stop zone exit

When a stable `SIGNAL_NOCHANGE` is detected while the decoder is in `SIGNAL_STOP`, the signal state transitions to `SIGNAL_FREE`. This prevents the decoder from remaining stuck without a red signal.

`speed_limit = 0` is only allowed while `SIGNAL_STOP` is active.

## DCC Memory programming

Each accepted DCC Memory message updates the active configuration in RAM immediately and reloads a one-second delay before saving to Flash. This includes immediate changes to the decoder address. Existing message confirmation and duplicate rejection remain unchanged. Rejected messages and invalid memory indexes do not reload the delay.

When a configuration write is pending and the one-second delay expires, the decoder saves its operating state using the existing backup and writes the configuration to Flash, then reads it back. It compares the read-back configuration with a copy of the requested RAM values. The write is not conditioned on the message queue or DMA state.

If the comparison fails, the decoder restores the requested RAM values and performs one additional write and read. It then restarts regardless of the retry outcome. There is no error state or dedicated error LED indication. Parameters calculated only at startup take effect after this restart. A power loss before saving discards unsaved RAM changes. Flash writing and restarting can temporarily interrupt DCC reception and motor control.

## Confirmed message history

The decoder retains the last 16 confirmed messages in a circular RAM buffer, including addressed and broadcast messages. Each entry contains the raw message bytes and their length. Unconfirmed messages, rejected addresses, and ignored duplicates are not recorded. The write index identifies the next entry to overwrite and wraps after 16 entries. There is no entry count. The history is cleared at restart; entries with zero length have not yet been filled.

## Uplink restart

When the uplink is stopped after 400 received DCC messages without an addressed or broadcast message, the stored message history is reset. The next matching message is therefore processed even if its content is identical to the last matching message received before the stop.

A broadcast message updates the communication LED and resets the received-message counter, but does not start the uplink.
