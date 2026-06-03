package com.hcmuaf.hydroponic.irrigation

import com.hcmuaf.hydroponic.data.IrrigationStatus
import java.util.Calendar
import java.util.TimeZone

/**
 * Mirrors web logic: +07:00, active 06:00–16:10, 10 min water / 30 min rest (2400 s cycle).
 * Outside window → night rest. Prefer server [get-irrigation-status.php] when available.
 */
object IrrigationStatusCalculator {
    private val ZONE: TimeZone = TimeZone.getTimeZone("Asia/Ho_Chi_Minh")
    private const val WATER_SECONDS = 10 * 60
    private const val CYCLE_SECONDS = 40 * 60

    fun computeNow(): IrrigationStatus {
        val cal = Calendar.getInstance(ZONE)
        val sod =
            cal.get(Calendar.HOUR_OF_DAY) * 3600 +
                cal.get(Calendar.MINUTE) * 60 +
                cal.get(Calendar.SECOND)
        val windowStart = 6 * 3600
        val windowEnd = 16 * 3600 + 10 * 60
        if (sod < windowStart || sod >= windowEnd) {
            return IrrigationStatus(
                label = "Nghỉ ban đêm (ngoài khung tưới)",
                code = "night",
            )
        }
        val sinceWindow = sod - windowStart
        val phase = ((sinceWindow % CYCLE_SECONDS) + CYCLE_SECONDS) % CYCLE_SECONDS
        return if (phase < WATER_SECONDS) {
            IrrigationStatus(label = "Đang tưới (10 phút)", code = "watering")
        } else {
            IrrigationStatus(label = "Nghỉ giữa chu kỳ (30 phút)", code = "rest30")
        }
    }
}
