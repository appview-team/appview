require("dotenv").config();

module.exports = {
  siteMetadata: {
    title: "AppView",
    titleTemplate: "%s · Instrument, Collect, Observe",
    description:
      "AppView is an open source instrumentation utility for any application, regardless of its runtime, with no code modification required. Collect only the data you need for full observability of your applications, systems and infrastructure.",
    url: "https://team-appview.github.io/appview",
    siteUrl: "https://team-appview.github.io/appview",
    image: "/images/isoarchitecture.png",
    twitterUsername: "@cribl",
  },
  plugins: [
    "gatsby-plugin-sass",
    "gatsby-plugin-sharp",
    "gatsby-plugin-react-helmet",
    "gatsby-plugin-sitemap",
    "gatsby-transformer-yaml",
    "gatsby-plugin-fontawesome-css",
    "gatsby-plugin-mdx",
    "gatsby-transformer-sharp",
    "gatsby-plugin-styled-components",
    {
      resolve: "gatsby-plugin-google-tagmanager",
      options: {
        id: "GTM-NNCJGH7",
        includeInDevelopment: false,
        defaultDataLayer: { platform: "gatsby" },
        routeChangeEventName: "gatsby-route-change",
      },
    },
    {
      resolve: "gatsby-plugin-manifest",
      options: {
        icon: "src/images/apple-touch-icon.png",
      },
    },
    {
      resolve: "gatsby-source-filesystem",
      options: {
        name: "code",
        path: "./src/pages/docs",
      },
    },
    {
      resolve: "gatsby-transformer-remark",
      options: {
        // Footnotes mode (default: true)
        // footnotes: true,
        // GitHub Flavored Markdown mode (default: true)
        gfm: true,
        // Disable pedantic mode, which supercedes gfm and is true by default
        pedantic: false,
        // Plugins configs
        plugins: [
          {
            resolve: `gatsby-remark-autolink-headers`,
            options: {
              enableCustomId: true,
            }
          },
          // `gatsby-remark-prismjs`,
          {
            resolve: "gatsby-remark-images",
            options: {
              maxWidth: 800,
            },
          },
        ],
      },
    },
    {
      resolve: "gatsby-source-filesystem",
      options: {
        name: "images",
        path: "./src/images/",
      },
      __key: "images",
    },
    {
      resolve: "gatsby-source-filesystem",
      options: {
        name: "pages",
        path: "./src/pages/",
      },
      __key: "pages",
    },
    {
      resolve: "gatsby-source-filesystem",
      options: {
        name: "docs",
        path: "./src/pages/docs",
      },
      __key: "markdown",
    },
    {
      resolve: "gatsby-source-filesystem",
      options: {
        path: "./src/data/",
      },
    },
    "gatsby-plugin-meta-redirect", // Should be last, per docs
  ],
};
