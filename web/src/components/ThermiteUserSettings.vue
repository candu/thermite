<template>
  <div class="thermite-user-settings">
    TODO: user settings
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
