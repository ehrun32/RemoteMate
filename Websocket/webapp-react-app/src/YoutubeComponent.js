import React, { Component } from "react";
import YouTube from "react-youtube";

export default class YoutubeComponent extends Component {
  videoOnReady(event) {
    // access to player in all event handlers via event.target
    event.target.pauseVideo();
    console.log(event.target);
  }
  render() {
    const opts = {
      height: "390",
      width: "640",
      playerVars: {
        // https://developers.google.com/youtube/player_parameters
        autoplay: 1,
      },
    };

    return (
      <YouTube videoId="n9Du-oESxCg" opts={opts} onReady={this.videoOnReady} />
    );
  }
}
