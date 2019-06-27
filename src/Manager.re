module D = Webapi.Dom;

open Types;

type value = {
  top: int,
  line: int,
  domNode: Dom.element,
  inlineValue: bool,
  popupValue: bool,
};

type state = {
  phr: phrase,
  value,
};

let state: ref(list(state)) = ref([]);

let attachToResultNode = childNode =>
  switch (D.Document.getElementById("result", D.document)) {
  | Some(parentNode) =>
    Js.log("actually doing somehting");
    parentNode |> D.Element.appendChild(childNode);
  | None => ()
  };

let createValueDomNode = (~top, ~value) => {
  open Webapi.Dom;
  let mainNode = document |> Document.createElement("div");

  mainNode
  |> Element.setAttribute(
       "style",
       "position: absolute; top:" ++ (top |> string_of_int) ++ "px",
     );
  let contentNode = document |> Document.createElement("span");
  mainNode |> Element.appendChild(contentNode);
  contentNode->Element.setInnerText(value);

  Js.log(mainNode);

  attachToResultNode(mainNode);

  mainNode;
};

let render = (~editor, ~phrs) => {
  let newState =
    phrs->Belt.List.map(phr => {
      let top = editor->CodeMirror.Editor.heightAtLine(~line=phr.startLine);
      Js.log(top);
      let value = {
        top,
        line: phr.startLine,
        domNode: createValueDomNode(~top, ~value=phr.value),
        inlineValue: false,
        popupValue: false,
      };
      {value, phr};
    });
  state := newState;
};
