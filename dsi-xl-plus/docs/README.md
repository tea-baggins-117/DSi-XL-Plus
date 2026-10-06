# DSi XL+ foundation

DSi XL+ is an optional enhancement of **TWiLight Menu++**, not a replacement launcher. This milestone adds bounded configuration, version identity, feature gates, config recovery foundations and a read-only settings information page. Development version: `0.1.0-dev`.

The foundation is compiled out by default (`XLPLUS=0`). Missing runtime configuration also defaults to disabled. Existing launch paths, themes, settings, saves and upstream attribution remain intact.

**Phase 1 acceptance requires successful pinned-Docker package builds and validated fallback/default behavior. Hardware smoke testing remains PENDING.** Host results alone do not satisfy build acceptance. No Phase 2 UI, Save Manager, dedicated Safe Mode, updater or Linux Manager is implemented.

- [Building](BUILDING.md), [architecture](ARCHITECTURE.md), [configuration](CONFIGURATION.md).
- [Recovery](RECOVERY.md), [Safe Mode boundary](SAFE_MODE.md), [testing](TESTING.md).
- [Development installation limits](INSTALLING.md), [updating](UPDATING.md), [Linux Manager](LINUX_MANAGER.md).
- [Licenses](LICENSES.md), [upstream integration record](../../UPSTREAM_INTEGRATION.md).
