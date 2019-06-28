%raw
{|require("./reason-mode.js");
require("codemirror/addon/selection/active-line");
|};

let editorConfig =
  CodeMirror.EditorConfiguration.make(
    ~value=
      {|let a = foo =>
  switch (foo) {
  | "1" => ()
  | _ => ()
  };

let b =
  fun
  | "1" => ()
  | _ => ();
  |},
    ~mode="reason",
    ~lineNumbers=true,
    ~viewportMargin=infinity,
    ~firstLineNumber=0,
    ~lineWrapping=true,
    ~styleActiveLine=true,
    ~gutters=[|"CodeMirror-linenumbers", "exec-gutter"|],
    (),
  );
let editor =
  CodeMirror.make(
    [%bs.raw "document.getElementById('editor')"],
    editorConfig,
  );

open Types;

editor->CodeMirror.Editor.onChange((_editor, diff) => {
  let fromPos = diff->CodeMirror.EditorChange.fromGet;
  let fromLine = fromPos->CodeMirror.Position.lineGet;

  EventEmitter.emit(EditedFromLine(fromLine));
});

let phrs = [
  {
    startLine: 0,
    endLine: 4,
    content: {|let a = foo =>
  switch (foo) {
  | "1" => ()
  | _ => ()
  };|},
    value: "let a: string => unit = <fun>;",
  },
  {
    startLine: 6,
    endLine: 9,
    content: {|let b =
  fun
  | "1" => ()
  | _ => ();|},
    value: "let b: string => unit = <fun>;",
  },
];

ReactDOMRe.renderToElementWithId(<ReactRoot editor phrs />, "result");

%raw
{|
  window.cm = editor;
|};
