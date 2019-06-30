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

let createGutterMarker = (handleHoverCb, status) => {
  open Webapi.Dom;
  open ExecutionState;

  let className =
    switch (status) {
    | Executed => "executed"
    | Executable => "executable"
    | ExecutableButIgnored => "executable-but-ignored"
    | NonExecutable => "non-executable"
    | ExecutableAndPlay => "executable-and-play"
    | NonExecutableAndPlay => "non-executable-and-play"
    };
  let marker = document |> Document.createElement("span");
  marker->Element.setClassName("exec-gutter " ++ className);

  marker |> Element.addMouseOverEventListener(handleHoverCb);
  marker;
};
type inlineWidget = option(CodeMirror.LineWidget.t);

type phrResult = {
  top: int,
  line: int,
  valueContent: string,
  inlineWidget,
  popupValue: bool,
};

type state = {
  phrResults: list(phrResult),
  gutterState: ExecutionState.gutterManager,
};

type action =
  | PhrsUpdated(list(phrase))
  | CalculateResultPosition
  | Editor_EditedFromLine(int)
  | ToggleInlineWidget(int)
  | UpdateGutterState(
      ExecutionState.gutterManager,
      list(ExecutionState.gutterPatch),
    )
  | HoverGutter(int);

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
  let doc =
    React.useMemo1(() => editor->CodeMirror.Editor.getDoc, [|editor|]);

  let getHeightAtLine = line => editor->CodeMirror.Editor.heightAtLine(~line);
  let (phrs, setPhrs) = React.useState(() => phrs);

  let (state, send) =
    ReactUpdate.useReducer(
      {
        gutterState: {
          maxLines: editor->CodeMirror.Editor.lineCount,
          lastExecutedLine: (-1),
          gutters: [||],
          requestToExecuteAtLine: (-1),
          playableLines: MutableIntSet.make(),
          phrs,
        },
        phrResults: mapPhrToPhrResult(~phrs, ~getHeightAtLine),
      },
      (action, state) =>
      switch (action) {
      | PhrsUpdated(phrs) =>
        Update({
          ...state,
          phrResults: mapPhrToPhrResult(~phrs, ~getHeightAtLine),
        })
      | CalculateResultPosition =>
        Update({
          ...state,
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
            ...state,
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
          ...state,
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
      | UpdateGutterState(gutterState, ops) =>
        UpdateWithSideEffects(
          {...state, gutterState},
          ({send}) => {
            editor->CodeMirror.Editor.operation((.) =>
              ops->Belt.List.forEach(
                fun
                | Patch_change(line, newStatus)
                | Patch_add(line, newStatus) =>
                  doc->CodeMirror.Doc.setGutterMarker(
                    ~line,
                    ~gutterId="exec-gutter",
                    ~value=
                      createGutterMarker(
                        _ => send(HoverGutter(line)),
                        newStatus,
                      ),
                  )
                | Patch_remove(line) => failwith("unimplemented"),
              )
            );
            None;
          },
        )
      | HoverGutter(line) =>
        let (newState, patch) =
          GutterManager.gutterEventHandler(
            ~event=ExecutionState.Ge_request_to_execute_at_line(line),
            ~state=state.gutterState,
          );
        SideEffects(
          ({send}) => {
            send(UpdateGutterState(newState, patch));
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

  React.useEffect1(
    () => {
      let (newState, patch) =
        GutterManager.calculateState(~phrs, ~state=state.gutterState);
      send(UpdateGutterState(newState, patch));
      None;
    },
    [||],
  );

  React.useEffect1(
    () => {
      EventEmitter.subscribe(event =>
        switch (event) {
        | UpdatePhrs(phrs) => setPhrs(_ => phrs)
        | EditedFromLine(line) => send(Editor_EditedFromLine(line))
        | CursorMove(line) => send(HoverGutter(line))
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
  </>;
};
