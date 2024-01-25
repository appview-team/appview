# Contribution Guidelines

Contribution to the project is very welcome and always needed. Please see the below guidelines for contributing to the project. If you have more questions, please feel free to join us on [slack](https://cribl.io/community/#form).

## :exclamation: Licensing

The Apache license (4a) requires that the original top level LICENSE file is preserved, but names can be added to the copyright owners list in that file. Copyright is automatic, but idenfifying key contributors is useful.

> Summary: In the LICENCE file, add names if you wish to; Never remove anything.

In addition, the license requires (4b) that all files changed carry a notice stating that You made a change.  The general method used to state that a file was changed is to add a new copyright line under the original one. 

> Summary: If you are changing a source code file you Must add or update this notice:

```
/*
 * (c) 2021 Cribl, Inc
 * (c) <Years of contribution to This file> Your Name
 */
```

The license also requires that all existing copyright notices are preserved.

## GitHub

### Issues

Should always contain:
- Issue detail
- Resolution detail

__Please tag issues appropriately for tracking purposes.__

### Pull Requests

Should contain:
- QA Instructions 
- ...anything else you find useful i.e. notes, todo list
- ...maybe some comments from the review process

### Discussions

Can contain:
- Proposals
- Ideas
- Q&A

### Security

Only AppView team members can approve and merge PR's.

AppView team members must have MFA on their GitHub account ; and use a GPG key to sign commits. 

## Coding Style

TBD - Please see the open [discussion](https://github.com/criblio/appview/discussions/1245) on coding style.

## Testing

Every PR should be tested in some way, sometimes by a manual QA reviewer, but preferably by an extension to integration or unit tests.

Every PR must pass the GitHub Actions CI pipeline 100%, which includes build, unit test, and integration test steps.

### Integration Tests

We use these to test features and should be the focal point of testing. 

Integration tests live in `test/integration/`.

### Unit Tests

We use these to test subsets of features where appropriate.

Unit Tests for the C code live in `test/unit/`.

Unit Tests for the Go code live in `cli/__package__/`.

## Documentation

With every release, we should update user documentation. User documentation lives in the `website` directory, and is published to [appview.dev](https://appview.dev).

## Compatibility

Please keep in mind the current set of [limitations and requirements](https://appview.dev/docs/requirements) for using AppView. We want to ensure that we maintain or shrink these where possible, and avoid growing them.
For example, we want to AppView to work on all popular linux distributions and versions; on glibc and musl environments ; with apps written in any language, etc.

We want to maintain backwards compatibility with previous versions of AppView in almost all cases.

We want to maintain our current licensing terms found in `LICENSE`.

## Branching & Tagging

![Branching](images/branching.png)

* The default branch is `master`.

* We create branches from the default branch for defects `bug/123-name` and
  feature requests `feature/123-name`. The number prefix is an issue number
  and the `-name` usually aligns with the issue title. Occasionally, we skip
  creating an issue and omit the number.

* We create PRs when work on issue branches is complete and ready to be
  reviewed and merged back into the default branch.

* We name releases using the typical `major.minor.maintenance` pattern for
  [Semantic Versioning](https://semver.org/). We start with `0.0.0` and
  increment...

  * ... `major` when breaking changes (BCs) are made.
  * ... `minor` when we add features without BCs.
  * ... `maintenance` for bug fixes without BCs.

  We append `-rc#` suffixes for release candidates.

* We create release candidate tags `v*.*.0-rc*` on the default branch as we
  approach the next major or minor release. When it's ready to go, we create
  the release tag `v*.*.0`, also on the default branch.

* Work toward a maintenance release is done on hotfix issue branches
  `hotfix/1234-name` created from the release branch, not the default branch.
  Like merges into the default branch, PRs are created to merge hotfixes back
  into the release branch when they're ready. Tags for the maintenance release
  candidates `v*.*.1-rc1` and the release itself `v*.*.1` are created on the
  release branch. Periodically, the release branch is merged back into the default branch.

* Minor releases after the `*.0.0` release are tagged on the default branch
  as usual — but after they're released, we merge them back out to the
  existing release branch. This blocks further maintenance releases for the
  prior minor release and provides a starting point for maintenance releases
  to the new minor release.

## Workflows

We use GitHub Workflows and Actions to automate CI/CD tasks for the project
when changes are made in the repository.

See [MAINTAINERS.md](MAINTAINERS.md) for more info.

## Container Images

We build and push container images to the
[`cribl/appview`](https://hub.docker.com/r/cribl/appview) and
[`cribl/appview-demo`](https://hub.docker.com/r/cribl/appview-demo)
repositories at Docker Hub. See [`docker/`](../docker/) for details on how
those images are built.

We currently build these for release `v*` tags and tag the images to match with
the leading `v` stripped off.

```text
docker run --rm -it cribl/appview:latest
```
or
```text
docker run --rm -it cribl/appview:1.1.3
```

## Tag Usage

`next`
- Users can pick up the latest master build
- Created automatically from master
- Available on docker

`latest`
- Users can pick up the latest release after announcement
- Created manually on Release Day
- Available on docker and the CDN

