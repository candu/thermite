<template>
  <v-navigation-drawer
    v-model="internalValue"
    app
    class="thermite-nav screen-height"
    :permanent="$vuetify.breakpoint.mdAndUp">
    <v-list dense nav>
      <v-list-item
        v-if="!$vuetify.breakpoint.mdAndUp"
        href="#internalState">
        <v-list-item-icon>
          <v-icon>mdi-thermometer</v-icon>
        </v-list-item-icon>
        <v-list-item-title>
          Status
        </v-list-item-title>
      </v-list-item>
      <v-list-item href="#weeklySchedule">
        <v-list-item-icon>
          <v-icon>mdi-calendar-week</v-icon>
        </v-list-item-icon>
        <v-list-item-title>
          Weekly Schedule
        </v-list-item-title>
      </v-list-item>
      <v-list-item href="#tempOverride">
        <v-list-item-icon>
          <v-icon>mdi-wallet-travel</v-icon>
        </v-list-item-icon>
        <v-list-item-title>
          Vacation Mode
        </v-list-item-title>
      </v-list-item>
      <v-list-item href="#dailySchedules">
        <v-list-item-icon>
          <v-icon>mdi-view-week</v-icon>
        </v-list-item-icon>
        <v-list-item-title>
          Daily Schedules
        </v-list-item-title>
      </v-list-item>
      <v-list-item href="#setPoints">
        <v-list-item-icon>
          <v-icon>mdi-temperature-celsius</v-icon>
        </v-list-item-icon>
        <v-list-item-title>
          Set Points
        </v-list-item-title>
      </v-list-item>
    </v-list>
  </v-navigation-drawer>
</template>

<script>
export default {
  name: 'ThermiteNav',
  props: {
    value: Boolean,
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
