<template>
  <div>
    <v-row>
      <v-col
        v-for="i in 4"
        :key="i"
        :cols="3">
        <ThermiteDailyScheduleEditor
          v-model="internalValue.dailySchedules[i - 1]"
          :set-points="internalValue.setPoints" />
      </v-col>
    </v-row>
  </div>
</template>

<script>
import ThermiteDailyScheduleEditor from '@/components/ThermiteDailyScheduleEditor.vue';

export default {
  name: 'ThermiteDailySchedules',
  components: {
    ThermiteDailyScheduleEditor,
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
