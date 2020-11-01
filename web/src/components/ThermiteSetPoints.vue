<template>
  <div>
    <v-row
      v-for="(setPoint, i) in internalValue.setPoints"
      :key="'setPoints_' + i">
      <v-col :cols="8">
        <v-text-field
          v-model="setPoint.name"
          label="Name">
          <template v-slot:prepend>
            <v-icon
              :color="SET_POINT_SYMBOLS[i].color">
              {{SET_POINT_SYMBOLS[i].icon}}
            </v-icon>
          </template>
        </v-text-field>
      </v-col>
      <v-col :cols="4">
        <v-text-field
          v-model.number="setPoint.tempTarget"
          :label="'Target Temp (\u00b0C)'"
          type="number" />
      </v-col>
    </v-row>
  </div>
</template>

<script>
import { SET_POINT_SYMBOLS } from '@/lib/Constants';

export default {
  name: 'ThermiteSetPoints',
  props: {
    value: Object,
  },
  data() {
    return { SET_POINT_SYMBOLS };
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
