# Maintainers

## Hosting

- The Repository is to be hosted on GitHub.
- The Website is to be hosted on GitHub by GitHub pages.
- Releases are to be hosted on GitHub on the Releases page.
- Docker images are to be hosted on Docker hub.
- The planning documentation is hosted on GitHub on the Discussions page.
- The CI runners are hosted on GitHub by GitHub actions which is configured to include:
  - The `build` workflow which is responsible for building the product, running unit tests, and integration tests, and reporting status.
  - The `website` workflow which is responsible for building and publishing the website.
  - The `release` workflow which is responsible for creating a release on GitHub Releases, and Docker hub.

## Release Updates

- A new release implies updates to:
  - The repository Releases page, to include a new release with the binaries for download.
  - The AppScope images on Docker hub.
  - The documentation Website.

#### Schedule

- Create a new maintenance release x.x.X for small changes and security patches
- Create a new minor release for bigger changes
- Create a new major release for changes that break backwards compatibility

#### Procedure

1. Ensure all changes are in the `master` branch. Perform all below actions from the `master` branch.
2. Ensure the CI build is green.
2. On the GitHub UI, in the Releases page, create a new release with the appropriate tag, and provide a description. This will trigger the workflow to create a new release, and update dockerhub.
3. On the GitHub UI, in the Actions page, click the button to generate an update to the website. This will trigger the workflow to create a new website build and publish it to GitHub pages

## Security Updates

Often it is necessary to perform security updates by bumping package or build versions.

To update the CLI:
- Modify the `go.mod` file to update the `go` version directive to the appropriate later version.
- Run the script `cli/bump_go_deps.sh` to update locked package versions.

