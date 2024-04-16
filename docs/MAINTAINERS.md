# Maintainers

## Hosting

- The Repository is to be hosted on GitHub.
- The Website is to be hosted on GitHub by GitHub pages.
- Releases are to be hosted on GitHub on the Releases page.
- Docker images are to be hosted on Docker hub.
- The planning documentation is hosted on GitHub on the Discussions page.
- The CI runners are hosted on GitHub by GitHub actions which is configured to include:
  - The `build-and-test` workflow which is responsible for building the product and running unit tests. This is automatically triggered to run on every non-draft PR branch; and on any push/merge to master.
  - The `update-website` workflow which is responsible for building and publishing the website. This is manually triggered (usually via the GitHub actions UI).
  - The `new-release` workflow which is responsible for creating a release on GitHub Releases, and Docker hub. This is automatically triggered when a new tag is created (usually via the GitHub releases UI).
  - The `integrations` workflow which is responsible for building the product, running unit tests, and running integration tests. This is manually triggered (usually via the GitHub actions UI).

## Releases

- A new release implies updates to:
  - The repository Releases page, to include a new release with the binaries for download.
  - The AppView images on Docker hub.
  - The documentation Website.

#### Release Schedule

- Create a new maintenance release x.x.X for small changes and security patches
- Create a new minor release for bigger changes
- Create a new major release for changes that break backwards compatibility

#### Release Procedure

1. Ensure all changes are in the `master` branch. Perform all below actions from the `master` branch.
2. Ensure the CI build and test stage passes (is green).
3. Update URLs to download AppView to reflect the version change in the README and on the docs website.
4. On the GitHub UI, in the Releases page, create a new release with the appropriate tag, and provide a description. This will trigger the `new-release` workflow to create a new release on GitHub and Docker hub.
5. On the GitHub UI, in the Actions page, click the button under the `update-website` workflow to generate an update to the website. This will trigger the workflow to create a new website build and publish it to GitHub pages

## Security Updates

Often it is necessary to perform security updates by bumping package or build versions.

To update the CLI:
- Modify the `go.mod` file to update the `go` version directive to the appropriate later version.
- Run the script `cli/bump_go_deps.sh` to update locked package versions.

