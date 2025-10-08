@Library(value=['corecomp-alsw-shared-pipelines@v25', 'corecomp-alsw-shared-library@v17'], changelog=false) _

DevelopmentPipeline_Main {
    buildenv = [
        docker_image_type: "generic",
        docker_image_version: "5.6",
        skip_schema_validation: true,
        environment_vars: [
            'ASPE_TEST_LOG_PATH': "\$WORKSPACE/ASPE_TESTING",
            'MUDP_STREAM_DEFINITIONS_PATH': "\$WORKSPACE/sources/auxiliary/F360_stream_definitions",
            'QT_QPA_PLATFORM': 'offscreen',
        ]
    ]
    dependencies = [
        system_packages: ["liblz4-dev","libgl1-mesa-glx","pkg-config","libcairo2-dev"],
        jfrog_files: [
            "corecomp_devops-aptiv-00000000-aspe-generic-local/logs/": "ASPE_TESTING/logs",
            "corecomp_devops-aptiv-00000000-aspe-generic-local/fixtures/": "ASPE_TESTING/fixtures",
        ],
        python_packages: [
            pip: "aspe_package/requirements.txt",
        ]
    ]
    build = [
        python: [
            main_folder: "aspe_package",
            common_options: "",
            targets: [:],
            test: [
                root_dir: "aspe_package",
                stage: "prebuild",
            ]
        ],

    ]
    testing = [
        use_googletest: "1.11.0",
        unittests: [],
        integrationtests: [],
    ]
    tics = [
        tics_config_url: "",
        project_name: "",
        branch_name: "main",
    ]

    post = [
        publisher: [
            files: ["sources/aspe_package/htmlcov/*"],
            html: [
                [reportName: "Coverage Report", reportDir: "sources/aspe_package/htmlcov", reportFiles: "index.html"],
            ],
        ]
    ]
}
