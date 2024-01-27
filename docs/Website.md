# AppView - Docs Website

_Using Node 16 or higher breaks some of this. It works with 14._

## Build on Github Actions
There is a script in `.github/workflows/website.yml` that will trigger the static build of the website.
See the script to understand how to trigger it (usually a tag or a button in github actions).
Essentially, the script will build the site, and copy the artifact to github self-hosting.
If the repository settings for 'Pages' is correct, the website will appear at the intended URL.

## Build Static files locally
```
website/deploy.sh
```
The files will be created in the `website/public` directory which is in `.gitignore`.

## Serve the Website locally
(requires nvm)
```
nvm install 14.18.1
nvm use 14.18.1
npm ci
npm run develop
```
Access the local development version of the website at [`http://127.0.0.1:8000/`](http://127.0.0.1:8000/).

## Configuration
URL, title, description are configured (and must be correct) in `gatsby-config.js`.
 
