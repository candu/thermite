<template>
  <div>
    <p>
I'm baby sartorial green juice irony squid air plant. Jean shorts synth narwhal helvetica plaid
poke four loko keffiyeh semiotics pug direct trade drinking vinegar gluten-free air plant.
Freegan pabst hexagon, hot chicken fashion axe cronut schlitz twee ethical jianbing green juice
organic godard master cleanse coloring book. Lo-fi stumptown austin cardigan poke, XOXO chia.
</p><p>
Authentic quinoa vinyl 90's aesthetic. Celiac bicycle rights crucifix, cliche brunch keffiyeh
shoreditch try-hard lyft deep v DIY. Polaroid glossier tote bag shoreditch tousled farm-to-table
tattooed. Fixie YOLO mumblecore venmo. Photo booth vaporware stumptown leggings blue bottle deep
v asymmetrical poke. Literally tbh tote bag lumbersexual you probably haven't heard of them
distillery chillwave. Hot chicken cliche iPhone, fashion axe taiyaki four dollar toast migas
godard wolf cloud bread flannel mustache blue bottle copper mug kitsch.
</p><p>
Kickstarter sustainable try-hard, vaporware ugh quinoa unicorn blog pour-over. Activated
charcoal poke tofu seitan. Twee photo booth kinfolk next level, pug raclette drinking vinegar
iPhone. Drinking vinegar kitsch austin neutra.
</p><p>
Ethical poutine affogato, jianbing austin tumeric fanny pack gluten-free disrupt tousled
wayfarers keytar drinking vinegar twee. Wayfarers snackwave tofu gluten-free vape shaman. Chambray
8-bit freegan coloring book. Photo booth tousled hot chicken venmo copper mug microdosing.
</p><p>
Pabst freegan tote bag, coloring book cloud bread gluten-free distillery try-hard chambray
lumbersexual helvetica truffaut subway tile neutra swag. Typewriter kale chips intelligentsia
master cleanse, pour-over kitsch pitchfork fashion axe iPhone wolf everyday carry gluten-free
austin migas. Marfa flannel tattooed distillery hot chicken banjo, kickstarter poke thundercats
DIY normcore vape occupy 90's butcher. Offal cardigan bushwick literally pitchfork. You probably
aven't heard of them next level salvia marfa cloud bread church-key affogato meh.
        </p>
  </div>
</template>

<script>
export default {
  name: 'ThermiteSetPoints',
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
