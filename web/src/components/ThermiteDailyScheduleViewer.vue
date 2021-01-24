<template>
  <div class="daily-schedule-viewer d-flex">
    <div
      v-for="(setPoint, i) in schedule"
      :key="i"
      class="daily-schedule-viewer-slot flex-grow-1 flex-shrink-1 lighten-2 pa-1"
      :class="SET_POINT_SYMBOLS[setPoint].color"
      :title="textTimes[i] + ': ' + textSetPoints[setPoint]">
    </div>
    <div
      v-if="weekday === nowWeekday"
      class="daily-schedule-viewer-now"
      :style="nowStyle">
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
    now: Date,
    setPoints: Array,
    weekday: Number,
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
    nowStyle() {
      const h = this.now.getHours();
      const m = this.now.getMinutes();
      const pct = 100 * (h / 60 + m / 1440);
      const pctFixed = pct.toFixed(2);
      const left = `calc(${pctFixed}% - 1px)`;
      return { left };
    },
    nowWeekday() {
      return this.now.getDay();
    },
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
  position: relative;

  & > div.daily-schedule-viewer-slot {
    cursor: pointer;
    opacity: 0.7;
    transition: all 100ms ease-in-out;
    user-select: none;
    &:hover {
      opacity: 1;
    }
  }

  & > div.daily-schedule-viewer-now {
    background-color: #000;
    height: calc(100% + 8px);
    position: absolute;
    top: -4px;
    width: 2px;
  }
}
</style>
