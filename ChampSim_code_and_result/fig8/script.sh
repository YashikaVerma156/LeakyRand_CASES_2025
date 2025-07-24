#Generate data for gnuplots.
python3 cdf_latest_hpca.py 0
python3 cdf_latest_hpca.py 1

#Generate plots.
gnuplot plot_cdf_sender_receiver_execution_time_train_suite_split.p
epstopdf sender-receiver-lr-cdf-train.eps
mv sender-receiver-lr-cdf-train.pdf fig8a.pdf

gnuplot plot_cdf_sender_receiver_execution_time_test_suite_split.p
epstopdf sender-receiver-lr-cdf-test.eps
mv sender-receiver-lr-cdf-test.pdf fig8b.pdf
