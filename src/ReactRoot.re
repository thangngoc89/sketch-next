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
        | UpdatePhrs(phrs) =>
          let _foo: unit = setPhrs(_ => phrs);
          ();
        | _ => ()
        }
      );
      None;
    },
    [||],
  );
  <>
    {values
     ->Belt.List.map(({top, valueContent}) =>
         <div
           key={string_of_int(top)}
           style={ReactDOMRe.Style.make(
             ~top={string_of_int(top) ++ "px"},
             ~position="absolute",
             (),
           )}>
           valueContent->React.string
         </div>
       )
     ->Belt.List.toArray
     ->React.array}
    <button
      style={ReactDOMRe.Style.make(
        ~bottom=[@reason.preserve_braces] "0px",
        ~position="absolute",
        (),
      )}
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
