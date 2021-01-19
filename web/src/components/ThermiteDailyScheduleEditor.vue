<template>
  <div>
     <v-text-field
        v-model="internalValue.name"
        label="Name"
        maxlength="15"
        messages="Maximum of 15 characters."
        required>
      </v-text-field>
      <div>
        <div
          v-for="(setPoint, i) in internalValue.schedule"
          :key="i">
          <div class="align-center d-flex">
            <div class="body-2 mr-2">{{times[i]}}</div>
            <v-select
              v-model="internalValue.schedule[i]"
              :class="SET_POINT_SYMBOLS[setPoint].color + ' lighten-4 pt-0'"
              hide-details
              :items="itemsSetPoints"
              :readonly="readonly">
              <template v-slot:selection="{ item }">
                <div class="align-center d-flex">
                  <v-icon
                    :color="SET_POINT_SYMBOLS[item.value].color">
                    {{SET_POINT_SYMBOLS[item.value].icon}}
                  </v-icon>
                  <span class="ml-2">{{item.text}}</span>
                </div>
              </template>
              <template v-slot:item="{ attrs, item, on }">
                <v-list-item v-on="on" v-bind="attrs">
                  <v-list-item-content>
                    <v-list-item-title>
                      <div class="align-center d-flex">
                        <v-icon
                          :color="SET_POINT_SYMBOLS[item.value].color">
                          {{SET_POINT_SYMBOLS[item.value].icon}}
                        </v-icon>
                        <span class="ml-2">{{item.text}}</span>
                      </div>
                    </v-list-item-title>
                  </v-list-item-content>
                </v-list-item>
              </template>
            </v-select>
          </div>
        </div>
      </div>
  </div>
</template>

<script>
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
    const times = new Array(48);
    for (let i = 0; i < 48; i++) {
      const h = Math.floor(i / 2);
      let hh = h.toString();
      if (h < 10) {
        hh = `0${h}`;
      }
      const mm = i % 2 === 0 ? '00' : '30';
      times[i] = `${hh}:${mm}`;
    }
    return { SET_POINT_SYMBOLS, times };
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
    itemsSetPoints() {
      return this.setPoints.map((setPoint, i) => {
        const { name, tempTarget } = setPoint;
        const text = `${name}: ${tempTarget} \u00b0C`;
        return { text, value: i };
      });
    },
  },
};
</script>
