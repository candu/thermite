<template>
  <div class="thermite-internal-state">
    <v-progress-circular
      v-if="internalState === null"
      indeterminate />
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
