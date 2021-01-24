<template>
  <div class="daily-schedule-viewer d-flex">
    <div
      v-for="(setPoint, i) in schedule"
      :key="i"
      :class="SET_POINT_SYMBOLS[setPoint].color + ' flex-grow-1 flex-shrink-1 lighten-2 pa-2'"
      :title="textTimes[i] + ': ' + textSetPoints[setPoint]">
    </div>
  </div>
</template>

<script>
import { SET_POINT_SYMBOLS } from '@/lib/Constants';
import DailySchedule from '@/lib/DailySchedule';

export default {
  name: 'ThermiteDailyScheduleViewer',
  props: {
    dailySchedule: Object,
    setPoints: Array,
  },
  data() {
    const textTimes = new Array(48);
    for (let i = 0; i < 48; i++) {
      const h = Math.floor(i / 2);
      let hh = h.toString();
      if (h < 10) {
        hh = `0${h}`;
      }
      const mm = i % 2 === 0 ? '00' : '30';
      textTimes[i] = `${hh}:${mm}`;
    }

    return {
      SET_POINT_SYMBOLS,
      textTimes,
    };
  },
  computed: {
    schedule() {
      const { schedule } = this.dailySchedule;
      return DailySchedule.decodeBytes(schedule);
    },
    textSetPoints() {
      return this.setPoints.map(
        ({ name, tempTarget }) => `${name}: ${tempTarget} \u00b0C`,
      );
    },
  },
};
</script>

<style lang="scss">
.daily-schedule-viewer {
  height: 32px;

  & > div {
    cursor: pointer;
    opacity: 0.7;
    transition: all 100ms ease-in-out;
    user-select: none;
    &:hover {
      opacity: 1;
    }
  }
}
</style>
