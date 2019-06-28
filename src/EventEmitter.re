open Types;

let cbs: ref(list(event => unit)) = ref([]);

let subscribe = cb => {
  cbs := [cb, ...cbs^];
};

let emit = event => {
  (cbs^)->Belt.List.forEach(cb => cb(event));
};
