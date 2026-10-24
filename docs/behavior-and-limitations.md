# Behavior and limitations

This is a **student prototype**, not a clinically tested triage or medical-alert device. The code does not run a trained AI/ML model: the "AI-assisted" branding from the original dashboard is a project title. The actual health score is rule-based.

## Preserved behavior

- Bed 1 reports MAX30102 pulse/SpO2, MLX90614 object temperature, and MPU6050 accelerometer motion.
- Beds 2 and 3 use **randomly simulated measurements** at each HTTP `/data` poll. Their names and measurements are not actual patients.
- OLED shows a 60-second baseline calibration and then the vital signs/status.
- A baseline-relative scoring heuristic can mark `CRITICAL` after sufficiently large differences.
- Emergency button and abrupt accelerometer movement produce five-second `WARNING` states, red LED, buzzer, and WhatsApp alert attempts.
- Green LED = `NORMAL`, yellow LED = `CALIBRATING`, red LED = `WARNING` or `CRITICAL`.
- The dashboard polls `/data` every two seconds and draws HR, SpO2, and temperature trends.

## Important limitations to address before use beyond a demo

1. **Not a medical device.** Neither SpO2, heart rate, object temperature, nor the health score is clinically validated here. A 50-sample MAX30102 window and readings obtained from a moving sensor may be unreliable.
2. **The alert thresholds are demo heuristics.** WARNING overrides CRITICAL for the duration of a button/motion warning, as in the original sketch. This is not suitable for real triage priority.
3. **A motion spike is not a verified fall.** Shaking the MPU6050 can trigger a fall warning, and a real fall might not.
4. **Old readings are retained.** The firmware keeps the last valid HR/SpO2 numbers after loss of contact. It does not expose measurement age or indicate stale data.
5. **Baseline fallbacks are synthetic.** If no valid readings exist after 60 seconds, it uses 75 bpm / 98% / 36.5 C for scoring.
6. **Network operations block.** Initial Wi-Fi connection and WhatsApp HTTP requests can block the loop; no offline queue, delivery confirmation, reconnect strategy, or watchdog is implemented.
7. **Only local-network HTTP.** The website and `/data` endpoint have no authentication or TLS. Use only an isolated trusted network with dummy names; do not expose it on the internet.
8. **API credentials are in firmware after building.** `.env` and generated headers are ignored by Git, but flashing embeds credentials. Rotation and device access controls still matter. HTTPS certificate verification is not explicitly configured in this prototype.
9. **Alerts are attempted, not guaranteed delivered.** The prototype marks an alert sent even if the API request fails. It sends a fresh alert on a new WARNING/CRITICAL transition, and repeated distinct events can re-alert.
10. **Trend buffer presentation.** The history buffer wraps every ten samples without chronological reordering. It is a demonstration chart, not a patient record.

Recommended future work: validity/age tracking, clinical review of thresholds, authenticated dashboard, dedicated fall-detection algorithm, asynchronous/network retry logic, encrypted storage if storing patient information, and clear synthetic/real measurement provenance.
