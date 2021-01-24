<template>
  <div>
    <v-checkbox
      v-model="enableTempOverride"
      label="Enable Vacation Mode?" />
    <v-text-field
      v-model.number="internalValue.tempOverride"
      :disabled="!enableTempOverride"
      :label="'Vacation Temp (\u00b0C)'"
      :max="30"
      :min="10"
      :step="0.25"
      type="number"
      required />
    <ThermiteDatetimePicker
      ref="overrideStart"
      v-model="internalValue.overrideStart"
      :disabled="!enableTempOverride"
      label="Vacation Mode Start" />
    <ThermiteDatetimePicker
      ref="overrideEnd"
      v-model="internalValue.overrideEnd"
      :disabled="!enableTempOverride"
      label="Vacation Mode End" />
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
  data() {
    const { overrideStart, overrideEnd } = this.value;
    const enableTempOverride = overrideStart !== 0 && overrideEnd !== 0;
    return { enableTempOverride };
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
  watch: {
    enableTempOverride() {
      if (this.enableTempOverride) {
        const now = DateTime.local();
        const nowPlusOneWeek = now.plus({ weeks: 1 });
        this.internalValue.overrideStart = Math.floor(now.toMillis() / 1000);
        this.internalValue.overrideEnd = Math.floor(nowPlusOneWeek.toMillis() / 1000);
      } else {
        this.$refs.overrideStart.clear();
        this.$refs.overrideEnd.clear();
      }
    },
  },
};
</script>
