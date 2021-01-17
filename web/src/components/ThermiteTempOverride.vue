<template>
  <div>
    <v-checkbox
      v-model="enableTempOverride"
      label="Enable Vacation Mode?" />
    <template v-if="enableTempOverride">
      <v-text-field
          v-model.number="internalValue.tempOverride"
          :label="'Vacation Temp (\u00b0C)'"
          :max="30"
          :min="10"
          :step="0.25"
          type="number"
          required />
      <ThermiteDatetimePicker
        v-model="internalValue.overrideStart"
        label="Vacation Mode Start" />
      <ThermiteDatetimePicker
        v-model="internalValue.overrideEnd"
        label="Vacation Mode End" />
    </template>
  </div>
</template>

<script>
import { DateTime } from 'luxon';

import ThermiteDatetimePicker from '@/components/ThermiteDatetimePicker.vue';

export default {
  name: 'ThermiteTempOverride',
  components: {
    ThermiteDatetimePicker,
  },
  props: {
    value: Object,
  },
  computed: {
    enableTempOverride: {
      get() {
        const { overrideStart, overrideEnd } = this.internalValue;
        return overrideStart !== 0 && overrideEnd !== 0;
      },
      set(enableTempOverride) {
        if (enableTempOverride) {
          const now = DateTime.local();
          const nowPlusOneWeek = now.plus({ weeks: 1 });
          this.internalValue.overrideStart = Math.floor(now.toMillis() / 1000);
          this.internalValue.overrideEnd = Math.floor(nowPlusOneWeek.toMillis() / 1000);
        } else {
          this.internalValue.overrideStart = 0;
          this.internalValue.overrideEnd = 0;
        }
      },
    },
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
