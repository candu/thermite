<template>
  <v-datetime-picker
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
export default {
  props: {
    value: Number,
  },
  computed: {
    internalValue: {
      get() {
        return new Date(this.value * 1000);
      },
      set(internalValue) {
        this.$emit('input', Math.floor(internalValue.valueOf() / 1000));
      },
    },
  },
};
</script>
