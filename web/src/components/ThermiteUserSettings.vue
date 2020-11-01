<template>
  <div
    class="thermite-user-settings"
    :class="{
      'md-and-up': $vuetify.breakpoint.mdAndUp,
    }">
    <v-progress-circular
      v-if="internalValue === null"
      color="primary"
      indeterminate />
    <template v-else>
      <section>
        <h2>Weekly Schedule</h2>
        <ThermiteWeeklySchedule
          v-model="internalValue"
          id="weeklySchedule" />
      </section>

      <section>
        <h2>Vacation Mode</h2>
        <ThermiteTempOverride
          v-model="internalValue"
          id="tempOverride" />
      </section>

      <section>
        <h2>Daily Schedules</h2>
        <ThermiteDailySchedules
          v-model="internalValue"
          id="dailySchedules" />
      </section>

      <section>
        <h2>Set Points</h2>
        <ThermiteSetPoints
          v-model="internalValue"
          id="setPoints" />
      </section>
    </template>
  </div>
</template>

<script>
import ThermiteDailySchedules from '@/components/ThermiteDailySchedules.vue';
import ThermiteSetPoints from '@/components/ThermiteSetPoints.vue';
import ThermiteTempOverride from '@/components/ThermiteTempOverride.vue';
import ThermiteWeeklySchedule from '@/components/ThermiteWeeklySchedule.vue';

export default {
  name: 'ThermiteUserSettings',
  components: {
    ThermiteDailySchedules,
    ThermiteSetPoints,
    ThermiteTempOverride,
    ThermiteWeeklySchedule,
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
