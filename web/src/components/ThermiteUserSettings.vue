<template>
  <div
    class="thermite-user-settings"
    :class="{
      'md-and-up': $vuetify.breakpoint.mdAndUp,
    }">
    <v-progress-circular
      v-if="internalValue === null"
      color="primary"
      indeterminate />
  </div>
</template>

<script>
export default {
  name: 'ThermiteUserSettings',
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

<style lang="scss">
.thermite-user-settings {
  &.md-and-up {
    margin-left: 256px;
  }
}
</style>
