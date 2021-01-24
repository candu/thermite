<template>
  <div class="weekly-schedule mt-4">
    <div
      v-for="(dailySchedule, i) in internalValue"
      :key="i"
      class="align-center d-flex pa-1">
      <span class="day-of-week flex-grow-0 flex-shrink-0">{{textDays[i]}}</span>
      <v-select
        v-model="internalValue[i]"
        class="select-daily-schedule flex-grow-0 flex-shrink-0 ml-4"
        :items="itemsDailySchedules" />
      <ThermiteDailyScheduleViewer
        class="flex-grow-1 flex-shrink-1 ml-2"
        :daily-schedule="dailySchedules[dailySchedule]"
        :now="now"
        :set-points="setPoints"
        :weekday="i" />
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
    const internalValue = toInternalValue(this.value);
    const now = new Date();

    const weekdays = Info.weekdays('short');
    const textDays = [
      weekdays[6],
      ...weekdays.slice(0, -1),
    ];
    return { internalValue, now, textDays };
  },
  computed: {
    itemsDailySchedules() {
      return this.dailySchedules.map(
        ({ name }, i) => ({ text: name, value: i }),
      );
    },
  },
  watch: {
    internalValue: {
      deep: true,
      handler() {
        const value = fromInternalValue(this.internalValue);
        this.$emit('input', value);
      },
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
    width: 40px;
  }
  & .select-daily-schedule {
    width: 200px;
  }
}
</style>
