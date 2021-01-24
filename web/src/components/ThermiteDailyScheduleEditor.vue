<template>
  <div class="mb-6">
     <v-text-field
        v-model="internalValue.name"
        label="Name"
        maxlength="15"
        messages="Maximum of 15 characters."
        required>
      </v-text-field>
      <div
        class="daily-schedule mt-4"
        :class="readonly ? '' : 'editable'">
        <div
          v-for="(setPoint, i) in internalValue.schedule"
          :key="i"
          @click="changeSchedule(i)">
          <div
            :class="SET_POINT_SYMBOLS[setPoint].color + ' lighten-4 pa-2'">
            <span>{{textTimes[i]}}</span>
            <v-icon
              class="ml-6"
              :color="SET_POINT_SYMBOLS[setPoint].color">
              {{SET_POINT_SYMBOLS[setPoint].icon}}
            </v-icon>
            <span class="ml-2">{{textSetPoints[setPoint]}}</span>
          </div>
        </div>
      </div>
  </div>
</template>

<script>
import Vue from 'vue';

import { SET_POINT_SYMBOLS } from '@/lib/Constants';

function toInternalValue(value) {
  const { name, schedule: scheduleRaw } = value;
  const schedule = new Array(48);
  scheduleRaw.forEach((b, i) => {
    /* eslint-disable no-bitwise */
    schedule[4 * i] = b & 0x3;
    schedule[4 * i + 1] = (b >> 2) & 0x3;
    schedule[4 * i + 2] = (b >> 4) & 0x3;
    schedule[4 * i + 3] = (b >> 6) & 0x3;
    /* eslint-enable no-bitwise */
  });
  return { name, schedule };
}

function fromInternalValue(internalValue) {
  const { name, schedule } = internalValue;
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
  return { name, schedule: scheduleRaw };
}

export default {
  name: 'ThermiteDailyScheduleEditor',
  props: {
    readonly: {
      type: Boolean,
      default: false,
    },
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
      if (this.readonly) {
        return;
      }

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
.daily-schedule.editable > div {
  cursor: pointer;
  opacity: 0.7;
  &:hover {
    opacity: 1;
  }
}
</style>
