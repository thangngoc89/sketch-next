%raw
{|require("./reason-mode.js")|};

let editorConfig =
  CodeMirror.EditorConfiguration.make(
    ~value="let a = 1;\n\n\n\nlet b = 2;",
    ~mode="reason",
    ~lineNumbers=true,
    ~viewportMargin=infinity,
    ~firstLineNumber=0,
    (),
  );
let editor =
  CodeMirror.make(
    [%bs.raw "document.getElementById('editor')"],
    editorConfig,
  );

open Types;

Manager.render(
  ~editor,
  ~phrs=[
    {startLine: 0, endLine: 0, content: "let a = 1;", value: "let a = 1;"},
    {startLine: 4, endLine: 4, content: "let b = 2;", value: "let b = 2;"},
  ],
);

%raw
{|
  window.cm = editor;
|};
