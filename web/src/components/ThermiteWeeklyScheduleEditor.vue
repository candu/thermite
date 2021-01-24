<template>
  <div class="weekly-schedule mt-4">
    <div
      v-for="(dailySchedule, i) in internalValue"
      :key="i"
      class="align-center d-flex pa-2">
      <span class="day-of-week">{{textDays[i]}}</span>
      <v-select
        v-model="internalValue[i]"
        class="select-daily-schedule flex-grow-0 flex-shrink-0"
        :items="itemsDailySchedules" />
      <ThermiteDailyScheduleViewer
        class="flex-grow-1 flex-shrink-1 ml-4"
        :daily-schedule="dailySchedules[dailySchedule]"
        :now="now"
        :set-points="setPoints" />
    </div>
  </div>
</template>

<script>
import { Info } from 'luxon';

import ThermiteDailyScheduleViewer from '@/components/ThermiteDailyScheduleViewer.vue';
import WeeklySchedule from '@/lib/WeeklySchedule';

function toInternalValue(value) {
  return WeeklySchedule.decode(value);
}

function fromInternalValue(internalValue) {
  return WeeklySchedule.encode(internalValue);
}

export default {
  name: 'ThermiteWeeklyScheduleEditor',
  components: {
    ThermiteDailyScheduleViewer,
  },
  props: {
    dailySchedules: Array,
    setPoints: Array,
    value: Number,
  },
  data() {
    const now = new Date();

    const weekdays = Info.weekdays('short');
    const textDays = [
      weekdays[6],
      ...weekdays.slice(0, -1),
    ];
    return { now, textDays };
  },
  computed: {
    internalValue: {
      get() {
        return toInternalValue(this.value);
      },
      set(internalValue) {
        const value = fromInternalValue(internalValue);
        this.$emit('input', value);
      },
    },
    itemsDailySchedules() {
      return this.dailySchedules.map(
        ({ name }, i) => ({ text: name, value: i }),
      );
    },
  },
  created() {
    this.intervalNow = window.setInterval(() => {
      this.now = new Date();
    }, 10000);
  },
  beforeDestroy() {
    window.clearInterval(this.intervalNow);
  },
};
</script>

<style lang="scss">
.weekly-schedule {
  & .day-of-week {
    width: 50px;
  }
  & .select-daily-schedule {
    width: 200px;
  }
}
</style>
