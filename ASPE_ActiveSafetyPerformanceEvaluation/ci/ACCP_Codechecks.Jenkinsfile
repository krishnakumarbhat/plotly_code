// Check our Confluence page to get more information about the tools and the different configuration options:
//  https://confluence.asux.aptiv.com/pages/viewpage.action?pageId=288321410
//
@Library(value=['corecomp-alsw-shared-pipelines@v25', 'corecomp-alsw-shared-library@v17'], changelog=false) _

DevelopmentPipeline_Codechecks {
    component_name = "ASPE_ActiveSafetyPerformanceEvaluation"
    buildenv = [
        docker_image_version: "5.6",
        skip_schema_validation: true,
    ]
    build = [:]
    testing = [
        use_googletest: "1.11.0",
    ]
    codeformatting = [
        ignore_results: ["buildifier", "cmake-format", "clang-format", "python-format"]
    ]
    coverity = [
        enabled: false,
        default_commit: false,
    ]
    cppcheck = [
        enabled: false,
    ]
    clang_tidy = [
        enabled: false,
    ]
    // Run CPD (Copy-Paste Detector): https://pmd.github.io/latest/pmd_userdocs_cpd.html
    cpd = [
        enabled: true,
        language: "python",
        analysis_path: "",
        excludes: []
    ]
    iwyu = [
        enabled: false,
    ]
    valgrind = [
        enabled: false
    ]
    cloc = [
        enabled: true
    ]
    lizard = [
        enabled: true
    ]
    pythonlint = [
        enabled: true,
        config: "aspe_package/pyproject.toml",
        thresholds: [
            [type: "TOTAL", threshold: 1522],
        ]
    ]
    cpplint = [
        enabled: false
    ]
}
