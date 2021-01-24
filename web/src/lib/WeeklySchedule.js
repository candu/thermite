class WeeklySchedule {
  static decode(weeklyScheduleRaw) {
    return [
      /* eslint-disable no-bitwise */
      weeklyScheduleRaw & 0x3,
      (weeklyScheduleRaw >> 2) & 0x3,
      (weeklyScheduleRaw >> 4) & 0x3,
      (weeklyScheduleRaw >> 6) & 0x3,
      (weeklyScheduleRaw >> 8) & 0x3,
      (weeklyScheduleRaw >> 10) & 0x3,
      (weeklyScheduleRaw >> 12) & 0x3,
      /* eslint-enable no-bitwise */
    ];
  }

  static encode(weeklySchedule) {
    /* eslint-disable no-bitwise */
    return weeklySchedule[0]
      | (weeklySchedule[1] << 2)
      | (weeklySchedule[2] << 4)
      | (weeklySchedule[3] << 6)
      | (weeklySchedule[4] << 8)
      | (weeklySchedule[5] << 10)
      | (weeklySchedule[6] << 12);
    /* eslint-enable no-bitwise */
  }
}

export default WeeklySchedule;
