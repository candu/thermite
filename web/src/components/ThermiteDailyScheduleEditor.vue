<template>
  <div class="mb-6">
     <v-text-field
        v-model="internalValue.name"
        label="Name"
        maxlength="15"
        messages="Maximum of 15 characters."
        required>
      </v-text-field>
      <div class="daily-schedule mt-4">
        <div
          v-for="(setPoint, i) in internalValue.schedule"
          :key="i"
          :class="SET_POINT_SYMBOLS[setPoint].color + ' lighten-4 pa-2'"
          @click="changeSchedule(i)">
          <span>{{textTimes[i]}}</span>
          <v-icon
            class="ml-4"
            :color="SET_POINT_SYMBOLS[setPoint].color">
            {{SET_POINT_SYMBOLS[setPoint].icon}}
          </v-icon>
          <span class="ml-2">{{textSetPoints[setPoint]}}</span>
        </div>
      </div>
  </div>
</template>

<script>
import Vue from 'vue';

import { SET_POINT_SYMBOLS } from '@/lib/Constants';
import DailySchedule from '@/lib/DailySchedule';

function toInternalValue(value) {
  const { name, schedule: scheduleRaw } = value;
  const schedule = DailySchedule.decodeBytes(scheduleRaw);
  return { name, schedule };
}

function fromInternalValue(internalValue) {
  const { name, schedule } = internalValue;
  const scheduleRaw = DailySchedule.encodeBytes(schedule);
  return { name, schedule: scheduleRaw };
}

export default {
  name: 'ThermiteDailyScheduleEditor',
  props: {
    setPoints: Array,
    value: Object,
  },
  data() {
    const internalValue = toInternalValue(this.value);

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
      internalValue,
      SET_POINT_SYMBOLS,
      textTimes,
    };
  },
  computed: {
    textSetPoints() {
      return this.setPoints.map(
        ({ name, tempTarget }) => `${name}: ${tempTarget} \u00b0C`,
      );
    },
  },
  methods: {
    changeSchedule(i) {
      let setPoint = this.internalValue.schedule[i];
      setPoint = (setPoint + 1) % 4;
      Vue.set(this.internalValue.schedule, i, setPoint);

      const value = fromInternalValue(this.internalValue);
      this.$emit('input', value);
    },
  },
};
</script>

<style lang="scss">
.daily-schedule > div {
  cursor: pointer;
  opacity: 0.7;
  transition: all 100ms ease-in-out;
  user-select: none;
  &:hover {
    opacity: 1;
  }
}
</style>
