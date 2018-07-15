//
//  ActionsController.swift
//  CurtainsiOS
//
//  Created by Tieme van Veen on 15/07/2018.
//  Copyright © 2018 Tieme van Veen. All rights reserved.
//

import UIKit
import Alamofire

class ActionsController: RequestController {

    override func viewDidLoad() {
        super.viewDidLoad()
    }
    
    @IBAction func onOpenFull(_ sender: Any) {
        self.sendOpenRequest(steps: .full, onRetry: #selector(onOpenFull(_:)))
    }
    
    @IBAction func onOpen34(_ sender: Any) {
        self.sendOpenRequest(steps: .threeQuarters, onRetry: #selector(onOpen34(_:)))
    }
    
    @IBAction func onOpen12(_ sender: Any) {
        self.sendOpenRequest(steps: .half, onRetry: #selector(onOpen12(_:)))
    }
    
    @IBAction func onOpen14(_ sender: Any) {
        self.sendOpenRequest(steps: .quarter, onRetry: #selector(onOpen14(_:)))
    }
    
    @IBAction func onStop(_ sender: Any) {
        self.onLoading(status: "Loading...")
        
        let url = "http://192.168.2.22/stop";
        Alamofire.request(url)
            .validate()
            .responseJSON { response in
                print("request \(response.response!.statusCode) - \(response.request!.url!) \(String(describing: response.result.value))");

                switch response.result {
                case .success:
                    self.onSuccess(status: "Stopping motor")
                case .failure(let err):
                    self.onFailure(status: "Request failed... \(err.localizedDescription)", action: #selector(self.onStop(_:)))
                }
        }
    }
    
    func sendOpenRequest(steps: Steps, onRetry: Selector) {
        self.onLoading(status: "Loading...")

        let url = "http://192.168.2.22/open"
        Alamofire.request(url, parameters: [ "amount": steps.rawValue ])
            .validate()
            .responseJSON { response in

                print("request \(response.response!.statusCode) - \(response.request!.url!) \(String(describing: response.result.value))");

                switch response.result {
                case .success:
                    self.onSuccess(status: "Opening curtain!")
                case .failure(let err):
                    self.onFailure(status: "Request failed... \(err.localizedDescription)", action: onRetry)
                }
        }
    }
}
