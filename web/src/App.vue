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

        <div
          v-if="userSettings !== null && dirty"
          class="mr-6">
          <v-btn
            class="mr-2"
            :disabled="loading"
            :loading="loading"
            outlined
            @click="actionCancel">
            Cancel
          </v-btn>

          <v-btn
            class="mr-2 white primary--text"
            :disabled="loading"
            :loading="loading"
            @click="actionSave">
            Save
          </v-btn>
        </div>
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
import Vue from 'vue';

import ThermiteInternalState from '@/components/ThermiteInternalState.vue';
import ThermiteNav from '@/components/ThermiteNav.vue';
import ThermiteUserSettings from '@/components/ThermiteUserSettings.vue';

async function getJson(url) {
  const response = await fetch(url);
  return response.json();
}

async function putJson(url, data) {
  const options = {
    body: JSON.stringify(data),
    headers: {
      'Content-Type': 'application/json',
    },
    method: 'PUT',
  };
  const response = await fetch(url, options);
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
      dirty: false,
      internalState: null,
      loading: false,
      showNav: this.$vuetify.breakpoint.mdAndUp,
      userSettings: null,
    };
  },
  watch: {
    userSettings: {
      deep: true,
      handler(userSettings, userSettingsOld) {
        if (userSettingsOld === null) {
          return;
        }
        this.dirty = true;
      },
    },
  },
  created() {
    this.loadAsync();
  },
  methods: {
    async actionCancel() {
      this.loading = true;

      const { backendIpAddress } = this;
      const userSettings = await getJson(`http://${backendIpAddress}/userSettings`);
      this.userSettings = userSettings;

      Vue.nextTick(() => {
        this.dirty = false;
        this.loading = false;
      });
    },
    async actionSave() {
      this.loading = true;

      const { backendIpAddress } = this;
      const userSettings = await putJson(`http://${backendIpAddress}/userSettings`, this.userSettings);
      this.userSettings = userSettings;

      Vue.nextTick(() => {
        this.dirty = false;
        this.loading = false;
      });
    },
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
