import * as React from "react";
import Header from "../components/Header";
import Hero from "../components/Hero";
import Highlights from "../components/Highlights";
import MobileHeader from "../components/MobileHeader";
import Footer from "../components/Footer";
import WhyAppView from "../components/WhyAppView";
import HowItWorks from "../components/HowItWorks";
import GetStarted from "../components/GetStarted";
import SEO from "../components/SEO";
import { Helmet } from "react-helmet";
const IndexPage = () => {
  return (
    <main>
      <SEO />
      <Helmet>
        <meta name="og:image" content={'https://cribl.io/wp-content/uploads/2022/01/thumb.appAppView.fullColorWhiteAlt.png'} />
      </Helmet>
      {/* <Alert /> */}
      <div className="display-xs">
        <MobileHeader />
      </div>

      <div className="display-md">
        <Header />
      </div>
      <Hero />
      <Highlights />
      <WhyAppView />
      <HowItWorks />
      <GetStarted />
      <Footer />
    </main>
  );
};

export default IndexPage;
