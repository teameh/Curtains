//
//  ViewController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 07/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit
import Alamofire

class ViewController: UIViewController {

    @IBOutlet weak var datePicker: UIDatePicker!
    @IBOutlet weak var saveButton: UIButton!
    @IBOutlet weak var statusLog: UITextView!
    
    override func viewDidLoad() {
        super.viewDidLoad()
        statusLog.text = "";
        // Do any additional setup after loading the view, typically from a nib.
    }

    @IBAction func onSave(_ sender: UIButton) {
        var intervalSinceNow: Int = Int(datePicker.date.timeIntervalSinceNow);
        self.statusLog.text = "Saving... " + "\n" + self.statusLog.text;
        
        if(intervalSinceNow <= 0) {
            self.statusLog.text = "intervalSinceNow <= 0... set to 5" + "\n" + self.statusLog.text;
            intervalSinceNow = 5;
        }
        
        Alamofire.request("http://192.168.2.22/steps/40000/4000/2500/\(intervalSinceNow)/").responseJSON { response in
            print("Request: \(String(describing: response.request))")   // original url request
            print("Response: \(String(describing: response.response))") // http url response
            print("Result: \(response.result)")                         // response serialization result
            
            self.statusLog.text = "Response: \(String(describing: response.response))" + "\n" + self.statusLog.text;
            
            if let json = response.result.value {
                print("JSON: \(json)") // serialized json response
                self.statusLog.text = "\(json)" + "\n" + self.statusLog.text;
            }
        }
    }
}

