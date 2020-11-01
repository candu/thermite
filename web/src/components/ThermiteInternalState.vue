<template>
  <div class="thermite-internal-state d-flex">
    <v-progress-circular
      v-if="internalState === null"
      indeterminate />
    <template v-else>
      <div class="mr-6">
        <div class="subtitle-2">Heater on?</div>
        <div class="headline">
          <template v-if="internalState.heater">
            <v-icon left>mdi-radiator</v-icon>
            <span>Yes</span>
          </template>
          <template v-else>
            <v-icon left>mdi-radiator-off</v-icon>
            <span>No</span>
          </template>
        </div>
      </div>
      <div class="mr-6">
        <div class="subtitle-2">Current</div>
        <div class="headline">{{internalState.temp}} &#x00b0;C</div>
      </div>
      <div>
        <div class="subtitle-2">Target</div>
        <div class="headline">{{internalState.tempTarget}} &#x00b0;C</div>
      </div>
    </template>
  </div>
</template>

<script>
export default {
  name: 'ThermiteInternalState',
  props: {
    internalState: Object,
  },
};
</script>
