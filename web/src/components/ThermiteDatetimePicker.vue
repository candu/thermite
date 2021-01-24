<template>
  <v-datetime-picker
    ref="datetimePicker"
    v-model="internalValue"
    v-bind="$attrs">
    <template v-slot:dateIcon>
      <v-icon>mdi-calendar</v-icon>
    </template>
    <template v-slot:timeIcon>
      <v-icon>mdi-clock-outline</v-icon>
    </template>
  </v-datetime-picker>
</template>

<script>
function toInternalValue(value) {
  if (value === 0) {
    return null;
  }
  return new Date(value * 1000);
}

function fromInternalValue(internalValue) {
  if (internalValue === null) {
    return 0;
  }
  return Math.floor(internalValue.valueOf() / 1000);
}

export default {
  props: {
    value: Number,
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
  },
  methods: {
    clear() {
      this.$refs.datetimePicker.clearHandler();
    },
  },
};
</script>
