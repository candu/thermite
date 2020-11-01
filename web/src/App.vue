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
          v-if="$vuetify.breakpoint.mdAndUp"
          :internal-state="internalState" />
        <v-app-bar-nav-icon
          v-else
          @click.stop="showNav = !showNav"></v-app-bar-nav-icon>
      </div>
    </v-app-bar>

    <v-main class="flex-grow-1 flex-shrink-1 fill">
      <ThermiteNav
        v-model="showNav" />
      <section
        v-if="!$vuetify.breakpoint.mdAndUp"
        class="pa-4"
        id="internalState">
        <h2>Status</h2>
        <ThermiteInternalState
          :internal-state="internalState" />
      </section>
      <ThermiteUserSettings
        v-model="userSettings"
        class="pa-4" />
    </v-main>
  </v-app>
</template>

<script>
import ThermiteInternalState from '@/components/ThermiteInternalState.vue';
import ThermiteNav from '@/components/ThermiteNav.vue';
import ThermiteUserSettings from '@/components/ThermiteUserSettings.vue';

async function getJson(url) {
  const response = await fetch(url);
  return response.json();
}

export default {
  name: 'App',
  components: {
    ThermiteInternalState,
    ThermiteNav,
    ThermiteUserSettings,
  },
  data() {
    return {
      backendIpAddress: '192.168.0.16',
      internalState: null,
      showNav: this.$vuetify.breakpoint.mdAndUp,
      userSettings: null,
    };
  },
  created() {
    this.loadAsync();
  },
  methods: {
    async loadAsync() {
      const { backendIpAddress } = this;
      const [
        internalState,
        userSettings,
      ] = await Promise.all([
        getJson(`http://${backendIpAddress}/internalState`),
        getJson(`http://${backendIpAddress}/userSettings`),
      ]);
      this.internalState = internalState;
      this.userSettings = userSettings;
    },
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
