open Types;

let createWidget = html => {
  open Webapi.Dom;
  let widget = document |> Document.createElement("div");
  widget->Element.setClassName("widget__inlineValue");

  let content = document |> Document.createElement("span");
  widget |> Element.appendChild(content);
  content->Element.setInnerHTML(html);

  widget;
};

type valueState = {
  top: int,
  line: int,
  valueContent: string,
  inlineValue: bool,
  popupValue: bool,
};

[@react.component]
let make = (~editor, ~phrs: list(phrase)) => {
  let (phrs, setPhrs) = React.useState(() => phrs);

  let doc =
    React.useMemo1(() => editor->CodeMirror.Editor.getDoc, [|editor|]);

  let values =
    React.useMemo1(
      () =>
        phrs->Belt.List.map(phr => {
          let top =
            editor->CodeMirror.Editor.heightAtLine(~line=phr.startLine);
          {
            top,
            line: phr.startLine,
            valueContent: phr.value,
            inlineValue: false,
            popupValue: false,
          };
        }),
      [|phrs|],
    );

  React.useEffect1(
    () => {
      EventEmitter.subscribe(event =>
        switch (event) {
        | UpdatePhrs(phrs) => setPhrs(_ => phrs)
        | EditedFromLine(line) =>
          setPhrs(currentPhrs =>
            currentPhrs->Belt.List.keep(({startLine}) => startLine < line)
          )
        | _ => ()
        }
      );
      None;
    },
    [||],
  );
  <>
    {values
     ->Belt.List.map(({top, line, valueContent}) =>
         <ResultNode
           key={string_of_int(top)}
           resultContent=valueContent
           top
           onMouseOver={_ =>
             doc
             ->CodeMirror.Doc.addLineClass(
                 ~line,
                 ~where=`background,
                 ~className="CodeMirror-activeline-background",
               )
             ->ignore
           }
           onMouseOut={_ =>
             doc
             ->CodeMirror.Doc.removeLineClass(
                 ~line,
                 ~where=`background,
                 ~className="CodeMirror-activeline-background",
               )
             ->ignore
           }
           handleShowInline={() =>
             editor->CodeMirror.Editor.addLineWidget(
               ~line,
               ~element=createWidget(valueContent),
               ~options=
                 CodeMirror.LineWidget.options(
                   ~noHScroll=true,
                   ~coverGutter=false,
                   ~above=false,
                   ~showIfHidden=false,
                   ~handleMouseEvents=false,
                 ),
             )
           }
         />
       )
     ->Belt.List.toArray
     ->React.array}
    <button
      style={ReactDOMRe.Style.make(~bottom="0px", ~position="absolute", ())}
      onClick={_ =>
        EventEmitter.emit(
          UpdatePhrs([
            {
              startLine: 0,
              endLine: 0,
              content: "let a = 1;",
              value: "let a = 1;",
            },
          ]),
        )
      }>
      "Update phrs"->React.string
    </button>
  </>;
};
