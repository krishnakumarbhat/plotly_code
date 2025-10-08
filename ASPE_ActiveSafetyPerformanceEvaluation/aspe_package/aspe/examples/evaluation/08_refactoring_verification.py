from aspe.evaluation.API.resim import f360_resim_comparision, generate_resim_comparison_report

if __name__ == '__main__':
    mudp_stream_def_path = r"C:\Users\sj07nx\Downloads\stream_definitions"
    log_list = [
        r"C:\Users\sj07nx\Downloads\2_12_2_15\FTP402_TC4_TA_154934_001.dvl",
        r"C:\Users\sj07nx\Downloads\2_12_2_15\FTP402_TC4_TT_155253_001.dvl",
    ]

    subtitle = 'DEX-822 ASPE based refactoring verification'  # if None, header will not be added
    resim_extension_before = 'rRf360t4060306v204p50_2_12'
    resim_extension_after = 'rRf360t4100309v205p50_2_15'
    commit_id_before = '7a6b92f'
    commit_id_after = '9ba02c3'
    comments = 'Lorem ipsum sit dolor amet.'  # if None, comment section will not be added
    output_path = 'refactoring_verification_report.pdf'

    results = f360_resim_comparision(log_list, resim_extension_before, resim_extension_after,
                                     mudp_stream_def_path,
                                     distance_threshold=0.01,
                                     optimize_memory_usage=True,
                                     break_on_first_fail=False)

    generate_resim_comparison_report(subtitle=subtitle,
                                     resim_extension_before=resim_extension_before,
                                     resim_extension_after=resim_extension_after,
                                     commit_id_before=commit_id_before,
                                     commit_id_after=commit_id_after,
                                     log_list=log_list,
                                     results=results,
                                     comments=comments,
                                     output_path=output_path)
