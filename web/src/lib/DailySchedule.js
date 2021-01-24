class DailySchedule {
  static decodeBytes(scheduleRaw) {
    const schedule = new Array(48);
    scheduleRaw.forEach((b, i) => {
      /* eslint-disable no-bitwise */
      schedule[4 * i] = b & 0x3;
      schedule[4 * i + 1] = (b >> 2) & 0x3;
      schedule[4 * i + 2] = (b >> 4) & 0x3;
      schedule[4 * i + 3] = (b >> 6) & 0x3;
      /* eslint-enable no-bitwise */
    });
    return schedule;
  }

  static encodeBytes(schedule) {
    const scheduleRaw = new Array(12);
    for (let i = 0; i < 12; i++) {
      /* eslint-disable no-bitwise */
      let b = schedule[4 * i];
      b |= schedule[4 * i + 1] << 2;
      b |= schedule[4 * i + 2] << 4;
      b |= schedule[4 * i + 3] << 6;
      scheduleRaw[i] = b;
      /* eslint-enable no-bitwise */
    }
    return scheduleRaw;
  }
}

export default DailySchedule;
