%raw
{|require("./reason-mode.js")|};

let editorConfig =
  CodeMirror.EditorConfiguration.make(
    ~value="let a = 1;\n\n\n\nlet b = 2;",
    ~mode="reason",
    ~lineNumbers=true,
    ~viewportMargin=infinity,
    (),
  );
let editor =
  CodeMirror.make(
    [%bs.raw "document.getElementById('editor')"],
    editorConfig,
  );

%raw
{|
  window.cm = editor;
|};
