
(defn get-sample-flight-data []
  (file/read (file/open "../sample_data.json" :r) :all))

