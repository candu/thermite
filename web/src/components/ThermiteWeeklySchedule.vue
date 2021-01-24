<template>
  <ThermiteWeeklyScheduleEditor
    v-model="internalValue.weeklySchedule"
    :daily-schedules="internalValue.dailySchedules"
    :set-points="internalValue.setPoints" />
</template>

<script>
import ThermiteWeeklyScheduleEditor from '@/components/ThermiteWeeklyScheduleEditor.vue';

export default {
  name: 'ThermiteWeeklySchedule',
  components: {
    ThermiteWeeklyScheduleEditor,
  },
  props: {
    value: Object,
  },
  computed: {
    internalValue: {
      get() {
        return this.value;
      },
      set(internalValue) {
        this.$emit('input', internalValue);
      },
    },
  },
};
</script>
