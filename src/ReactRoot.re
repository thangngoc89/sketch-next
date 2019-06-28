open Types;

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
         <div
           key={string_of_int(top)}
           style={ReactDOMRe.Style.make(
             ~top={string_of_int(top) ++ "px"},
             ~width="300px",
             ~position="absolute",
             (),
           )}
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
           }>
           valueContent->React.string
         </div>
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
