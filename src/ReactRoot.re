open Types;

let createWidget = html => {
  open Webapi.Dom;
  let widget = document |> Document.createElement("div");
  widget->Element.setClassName("widget__inlineValue");

  let content = document |> Document.createElement("pre");
  widget |> Element.appendChild(content);
  content->Element.setInnerText(html);

  widget;
};

type inlineWidget = option(CodeMirror.LineWidget.t);

type phrResult = {
  top: int,
  line: int,
  valueContent: string,
  inlineWidget,
  popupValue: bool,
};

type state = {phrResults: list(phrResult)};

type action =
  | PhrsUpdated(list(phrase))
  | CalculateResultPosition
  | Editor_EditedFromLine(int)
  | ToggleInlineWidget(int);

let mapPhrToPhrResult = (~phrs, ~getHeightAtLine) => {
  phrs->Belt.List.map(phr =>
    {
      top: getHeightAtLine(phr.startLine),
      line: phr.startLine,
      valueContent: phr.value,
      inlineWidget: None,
      popupValue: false,
    }
  );
};

[@react.component]
let make = (~editor, ~phrs: list(phrase)) => {
  let getHeightAtLine = line => editor->CodeMirror.Editor.heightAtLine(~line);
  let (phrs, setPhrs) = React.useState(() => phrs);

  let (state, send) =
    ReactUpdate.useReducer(
      {phrResults: mapPhrToPhrResult(~phrs, ~getHeightAtLine)},
      (action, state) =>
      switch (action) {
      | PhrsUpdated(phrs) =>
        Update({phrResults: mapPhrToPhrResult(~phrs, ~getHeightAtLine)})
      | CalculateResultPosition =>
        Update({
          phrResults:
            state.phrResults
            ->Belt.List.map(phrResult =>
                {...phrResult, top: getHeightAtLine(phrResult.line)}
              ),
        })
      | Editor_EditedFromLine(editedFromLine) =>
        let needsCleanUp =
          state.phrResults
          ->Belt.List.keep(({line}) => line >= editedFromLine);

        UpdateWithSideEffects(
          {
            phrResults:
              state.phrResults
              ->Belt.List.keep(({line}) => line < editedFromLine),
          },
          _ => {
            needsCleanUp->Belt.List.forEach(({inlineWidget}) =>
              switch (inlineWidget) {
              | None => ()
              | Some(lwHandler) => CodeMirror.LineWidget.clear(lwHandler)
              }
            );
            None;
          },
        );

      | ToggleInlineWidget(line) =>
        let newState = {
          phrResults:
            state.phrResults
            ->Belt.List.map(phrResult =>
                if (line == phrResult.line) {
                  switch (phrResult.inlineWidget) {
                  | None => {
                      ...phrResult,
                      inlineWidget:
                        Some(
                          editor->CodeMirror.Editor.addLineWidget(
                            ~line,
                            ~element=createWidget(phrResult.valueContent),
                            ~options=
                              CodeMirror.LineWidget.options(
                                ~noHScroll=true,
                                ~coverGutter=false,
                                ~above=false,
                                ~showIfHidden=false,
                                ~handleMouseEvents=false,
                              ),
                          ),
                        ),
                    }
                  | Some(lineWidgetHandler) =>
                    CodeMirror.LineWidget.clear(lineWidgetHandler);
                    {...phrResult, inlineWidget: None};
                  };
                } else {
                  phrResult;
                }
              ),
        };
        UpdateWithSideEffects(
          newState,
          ({send}) => {
            send(CalculateResultPosition);
            None;
          },
        );
      }
    );

  React.useEffect1(
    () => {
      send(PhrsUpdated(phrs));
      None;
    },
    [|phrs|],
  );

  let doc =
    React.useMemo1(() => editor->CodeMirror.Editor.getDoc, [|editor|]);

  React.useEffect1(
    () => {
      EventEmitter.subscribe(event =>
        switch (event) {
        | UpdatePhrs(phrs) => setPhrs(_ => phrs)
        | EditedFromLine(line) => send(Editor_EditedFromLine(line))
        | _ => ()
        }
      );
      None;
    },
    [||],
  );
  <>
    {state.phrResults
     ->Belt.List.map(({top, line, valueContent, inlineWidget}) =>
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
           showInlineWidget={
             switch (inlineWidget) {
             | None => false
             | Some(_) => true
             }
           }
           handleToggleInlineWidget={() => send(ToggleInlineWidget(line))}
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
