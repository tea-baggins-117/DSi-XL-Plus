# Updating — future work

The Phase 10 update/rollback system is not implemented. Phase 1 does not download or apply console updates.

Future updates must stage and verify application files, retain a known-good application package, and preserve configuration/metadata and user saves separately. Application rollback must never roll back user save data. Newer configuration schemas remain preserved by this older foundation rather than being silently rewritten.

For development, build in fresh separate directories and preserve Phase 0. Follow the upstream integration record before merging new upstream revisions; do not confuse a changed native toolchain with an XL+ feature failure.
