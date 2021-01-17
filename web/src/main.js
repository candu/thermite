import Vue from 'vue';
import Vuelidate from 'vuelidate';
import DatetimePicker from 'vuetify-datetime-picker';

import App from '@/App.vue';
import vuetify from '@/plugins/vuetify';

Vue.config.productionTip = false;
Vue.use(Vuelidate);
Vue.use(DatetimePicker);

new Vue({
  vuetify,
  render: (h) => h(App),
}).$mount('#app');
