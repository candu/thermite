<template>
  <v-app class="d-flex flex-column screen-height">
    <v-app-bar
      app
      color="primary"
      dark>
      <div class="d-flex align-center full-width">
        <v-icon left>mdi-home-thermometer</v-icon>
        <h1>thermite</h1>

        <v-spacer></v-spacer>

        <ThermiteInternalState
          :internal-state="internalState" />
      </div>
    </v-app-bar>

    <v-main class="flex-grow-1 flex-shrink-1">
      <ThermiteUserSettings
        class="fill-height"
        :user-settings="userSettings" />
    </v-main>
  </v-app>
</template>

<script>
import ThermiteInternalState from './components/ThermiteInternalState.vue';
import ThermiteUserSettings from './components/ThermiteUserSettings.vue';

export default {
  name: 'App',
  components: {
    ThermiteInternalState,
    ThermiteUserSettings,
  },
  data() {
    return {
      internalState: null,
      userSettings: null,
    };
  },
};
</script>

<style lang="scss">
.full-height {
  height: 100%;
}

.full-width {
  width: 100%;
}

.screen-height {
  height: 100vh;
}
</style>
