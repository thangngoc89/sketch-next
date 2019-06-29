let maxLength = 40;

[@react.component]
let make = (~resultContent, ~top, ~onMouseOver, ~onMouseOut) => {
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
    <button className="result__button"> <Icons.FiEye /> </button>
    <button className="result__button"> <Icons.FiPlusSquare /> </button>
  </div>;
};
