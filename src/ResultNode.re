let maxLength = 40;

[@react.component]
let make =
    (~handleShowInline, ~resultContent, ~top, ~onMouseOver, ~onMouseOut) => {
  let (inlineStatus, setInlineStatus) = React.useState(() => false);
  let (inlineWidgetHandle, setInlineWidgetHandle) =
    React.useState(() => None);

  React.useEffect1(
    () => {
      if (inlineStatus) {
        setInlineWidgetHandle(_ => Some(handleShowInline()));
      } else {
        switch (inlineWidgetHandle) {
        | None => ()
        | Some(handle) =>
          CodeMirror.LineWidget.clear(handle);
          setInlineWidgetHandle(_ => None);
        };
      };
      None;
    },
    [|inlineStatus|],
  );

  let clippedContent =
    React.useMemo1(
      () => {
        let contentLength = Js.String.length(resultContent);

        if (contentLength < maxLength) {
          resultContent;
        } else {
          let newLineIndex = Js.String.indexOf("\n", resultContent);

          if (newLineIndex < maxLength) {
            Js.String.slice(~from=0, ~to_=newLineIndex, resultContent)
            ++ "...";
          } else {
            Js.String.slice(~from=0, ~to_=50, resultContent) ++ "...";
          };
        };
      },
      [|resultContent|],
    );
  <div
    className="result__container"
    style={ReactDOMRe.Style.make(
      ~top={
        string_of_int(top) ++ "px";
      },
      (),
    )}
    onMouseOver
    onMouseOut>
    <span className="result__content"> clippedContent->React.string </span>
    <button
      className="result__button"
      onClick={_ => setInlineStatus(current => !current)}>
      {!inlineStatus ? <Icons.FiEye /> : <Icons.FiEyeOff />}
    </button>
    <button className="result__button"> <Icons.FiPlusSquare /> </button>
  </div>;
};
