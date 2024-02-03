import React, { useState, useEffect } from "react";
import { FontAwesomeIcon } from "@fortawesome/react-fontawesome";
import "../../scss/_starCount.scss";
import "../../utils/font-awesome";

export default function StarCount() {
  const [starsCount, setStarsCount] = useState(0);

  useEffect(() => {
    fetch(`https://api.github.com/repos/appview-team/appview`)
      .then((response) => response.json())
      .then((resultData) => {
        setStarsCount(resultData.stargazers_count);
      });
  }, []);

  return (
    <a
      className="starCount-container"
      href="https://github.com/appview-team/appview"
    >
      <div className="gitLogo">
        <FontAwesomeIcon icon={["fab", "github-square"]} />
      </div>
      <div className="starCount">
        <FontAwesomeIcon icon={"star"} />
        <span className="count">
          {starsCount > 999 ? (starsCount / 1000).toFixed(1) + "K" : starsCount}
        </span>
      </div>
    </a>
  );
}
